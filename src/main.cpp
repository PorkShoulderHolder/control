#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp> 
#include <string>
#include "ui.h"
#include "camera_calibration.h"
#include "network_manager.h"
#include "bot.h"
#include "state.h"
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <map>
#include <cstdlib>
#include <csignal>

#include "argparse.hpp"
#include "behaviors.h"


#define DEBUG false
#define CONTROL 0
#define WATCH 1
#define PREDATOR_PREY 2
#define FOLLOW 3

#define RIGHT_ARROW_KEY 63235
#define LEFT_ARROW_KEY 63234
#define TOP_ARROW_KEY 63232

using namespace cv;
using namespace std;


std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 8, pipe) != NULL )
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }

    pclose(pipe);
    return result;
}

void exiting(int i){
    State *S = State::shared_instance();
    for( Bot *b : S->devices ){
        std::vector<MOTOR> off;
        off.push_back(M_RIGHT_OFF);
        off.push_back(M_LEFT_OFF);
        b->apply_motor_commands(off);
        delete b;
    }
    NetworkManager *manager = new NetworkManager();
    std::string msg = "{\"completed\":1}";
    char buffer[1024];
    std::string brain_host = "127.0.0.1";
    manager->send_tcp((char*)brain_host.c_str(), (char*)msg.c_str(), 9999, buffer);
    std::cout << "\n --- bye :^) --- \n" << std::endl;
    std::exit(0);
}

void handle_event(int key){
    State *S = State::shared_instance();
    Bot *bot;
    if(S->devices.size() > 0) {
        bot = S->devices.back();
        bot->training = true;
    }else {
        return;
    }
    switch(key) {

        case RIGHT_ARROW_KEY:{
            if (bot->command_queue.size() == 0) {
                std::vector<MOTOR> c = motor_instructions(C_LEFT_OFF_RIGHT_ON);
                bot->command_queue.push_front(c);
                bot->command_queue.push_front(c);
            }
            else if (bot->command_queue.size() >= 2){
                bot->command_queue[0][1] = M_RIGHT_ON;
                bot->command_queue[1][1] = M_RIGHT_ON;
            }
            else{
                bot->command_queue[0][1] = M_RIGHT_ON;
            }
            break;
        }
        case LEFT_ARROW_KEY:{
            if (bot->command_queue.size() == 0) {
                std::vector<MOTOR> c = motor_instructions(C_LEFT_ON_RIGHT_OFF);
                bot->command_queue.push_front(c);
                bot->command_queue.push_front(c);
            }
            else if (bot->command_queue.size() >= 2){
                bot->command_queue[0][0] = M_LEFT_ON;
                bot->command_queue[1][0] = M_LEFT_ON;
            }
            else{
                bot->command_queue[0][0] = M_LEFT_ON;
            }
            break;
        }
        case TOP_ARROW_KEY:{
            std::vector<MOTOR> c = motor_instructions(C_BOTH_ON);
            if (bot->command_queue.size() == 0) {
                bot->command_queue.push_front(c);
                bot->command_queue.push_front(c);
            }
            else if (bot->command_queue.size() >= 2){
                bot->command_queue[0] = c;
                bot->command_queue[1] = c;
            }
            else{
                bot->command_queue[0] = c;
            }
            break;
        }
        default:
            break;
    }
}

void generate_markers(int count){
    cv::Mat markerImage; 
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_1000);
    int i;

    const int width = (int)sqrt(count);
    cv::Mat display_image = cv::Mat::ones((count / 5) * 150 + 20, 760, CV_32F);
    int buffer = 10;
    for(i = 0 ;i < count; i++ ){

      cv::aruco::drawMarker(dictionary, i, 100, markerImage, 1);
      int x = (i % 5) * 150 + 10;
      int y = (int)(i / 5) * 150 + 20;
      std::string s = std::to_string(i);
      cv::Point p;
      p.x = x + 22;
      p.y = y + 3;
      cv::Rect r = cv::Rect(x + 20, y + 20, 100, 100);
      markerImage.copyTo(display_image(r));
      cv::putText(display_image, s, p, 1, 1, cv::Scalar(0, 0, 0), 1); 
    }
    cv::imshow("d", display_image);
}

void init_application(){

    cv::namedWindow(MAIN_WINDOW, cv::WINDOW_NORMAL);
    cv::namedWindow(INFO_WINDOW, cv::WINDOW_NORMAL);
}

std::string update_fps(int i){
    static std::string fps_str;
    int sample_pd = 80;
    static time_t start;
    static time_t finish;
    if (i == 1){
        time(&start);
    }
    if(i % sample_pd == 0){
        time(&finish);
        double seconds = difftime (finish, start);
        double fps = sample_pd / seconds;
        fps_str = std::string(to_string(fps)) + " fps";
        time(&start);
    }
    return fps_str;
}


void main_loop(int device_index, int main_mode, int sub_mode){
    cv::Point text_loc(20, 20);
    cv::Mat current_image;
    int i = 1;
    std::string fps_str = "0 fps";
    cv::VideoCapture input_stream(device_index);

    if (!input_stream.isOpened()) {
        std::cout << "Unable to read stream from specified device." << std::endl;
        return;
    }
    init_application();
    State *S = State::shared_instance(); // first init
    switch (sub_mode){
        case FOLLOW:
            S->behavior = new FollowTheLeader();
            S->behavior->bots = S->devices;
        case PREDATOR_PREY:
//            S->behavior = new PredatorPrey();
            S->behavior->bots = S->devices;
    }
    Window window;

    /* event loop */
    while(1){
        S = State::shared_instance();
        input_stream >> current_image;
        if(main_mode == CONTROL){
            S->update(current_image);
        }
        else if(main_mode == WATCH) {
            int k1 = waitKey(5);
            int k2 = waitKey(5);
            if (k1 >= 0)
                handle_event(k1);
            if (k2 != k1 && k2 >= 0)
                handle_event(k2);
            if ((k1 >= 0 || k2 >= 0) && S->devices.size() > 0) {
                std::vector<MOTOR> c = motor_instructions(C_BOTH_OFF);
                S->devices.back()->command_queue.push_back(c);
                S->devices.back()->command_queue.push_back(c);
            }
            S->update(current_image);
        }

        cv::Mat final_draw;
        cv::resize(S->display_image, final_draw, Size(612, 384));
        fps_str = update_fps(i);
        cv::putText(final_draw, fps_str, text_loc, 1, 1, cv::Scalar(155, 155, 0), 1);
        cv::imshow(MAIN_WINDOW, final_draw);
        cv::imshow(INFO_WINDOW, S->info_image);
        if(i == -1) break;
        i++;
    }
}

void test_network(){
    NetworkManager *manager = new NetworkManager();
    manager->send_udp((char *) "goodvibes1067796.local", (char *) "r:::1", 8888);
}

int main(int argc, const char** argv )
{
    std::map<std::string, int> config;
    config["follow"] = FOLLOW;
    config["hunt"] = PREDATOR_PREY;

    ArgumentParser parser;
    parser.addArgument("-r", "--run", '*');
//    parser.addArgument("-w", "--watch");
//    parser.addArgument("-c", "--camera");
    parser.parse((size_t)argc, argv);

    std::signal(SIGINT, exiting);
    srand (static_cast <unsigned> (time(0)));
    if (parser.exists("generate-markers")) {
        int count = 50;
        if (argc > 2) {
            count = atoi(argv[2]);
        }
        generate_markers(count);
    }
    else if(strcmp(argv[1], "calibrate_camera") == 0){
        calibrate_camera_main(argc, argv);
    }
    else{
        system("gtimeout 2 ./mdns-mod -B");
        int device_index = 0;


        if(parser.exists("camera")){
            device_index = parser.retrieve<int>("camera");
        }
        if(parser.exists("run")){
            int sub_mode = 0;
            vector<string> arg = parser.retrieve<vector<string> >("run");
            std::string a = arg[0];
            if(config.count(a) > 0){
                sub_mode = config[a];
            }
            else{
                std::cout << "unrecognized arg '" + a + "'" << std::endl;
            }
            main_loop(device_index, CONTROL, sub_mode);
        }
        else if(parser.exists("watch")) {
            main_loop(device_index, WATCH, 0);
        }
    }
    return 0;
}

