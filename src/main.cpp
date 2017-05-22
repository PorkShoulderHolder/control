#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp> 
#include <string>
#include "camera_calibration.h"
#include "network_manager.h"
#include "bot.h"
#include "state.h"
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <csignal>


#define DEBUG false
#define CONTROL 0
#define WATCH 1

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
    for( Bot *b : State::devices ){
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

void handle_keypress(int key){
    std::cout << key << std::endl;
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
    waitKey(0);
  
}

void init_application(){

    cv::namedWindow(MAIN_WINDOW, cv::WINDOW_NORMAL);
    cv::namedWindow(INFO_WINDOW, cv::WINDOW_NORMAL);
}

std::string update_fps(int i, time_t start, time_t finish){
    static std::string fps_str;
    static int sample_pd = 80;
    if(i % sample_pd == 0){
        time(&finish);
        double seconds = difftime (finish, start);
        double fps = sample_pd / seconds;
        fps_str = std::string(to_string(fps)) + " fps";
        time(&start);
    }
    return fps_str;
}

void main_loop(int device_index, int mode){
    cv::Point text_loc(20, 20);
    cv::Mat current_image;
    int i = 1;
    time_t start, finish;
    std::string fps_str = "0 fps";
    time(&start);
    cv::VideoCapture input_stream(device_index);

    if (!input_stream.isOpened()) {
        std::cout << "Unable to read stream from specified device." << std::endl;
        return;
    }
    init_application();
    init_state();
    while(1){
        input_stream >> current_image;
        if(mode == CONTROL){
            State::update(current_image);
        }
        else if(mode == WATCH){
            int key = waitKey(10);
            if(key >= 0){
                handle_keypress(key);
            }
        }

        cv::Mat final_draw;
        cv::resize(State::display_image, final_draw, Size(612, 384));
        fps_str = update_fps(i, start, finish);
        cv::putText(final_draw, fps_str, text_loc, 1, 1, cv::Scalar(155, 155, 0), 1);
        cv::imshow(MAIN_WINDOW, final_draw);
        if(waitKey(30) >= 0) break;
        i++;
    }
}

void test_network(){
    NetworkManager *manager = new NetworkManager();
    manager->send_udp((char *) "goodvibes1067796.local", (char *) "r:::1", 8888);
}

int main(int argc, char** argv )
{

    std::signal(SIGINT, exiting);
    srand (static_cast <unsigned> (time(0)));
    if (argc > 1 && strcmp(argv[1], "generate-markers") == 0){
        int count = 50;
        if(argc > 2){
            count = atoi(argv[2]);
        }
        generate_markers(count);
    }
    else if (argc > 1 && strcmp(argv[1], "run-tests") == 0){

        if( argc > 3) {
            Bot *b = new Bot(argv[2]);
            std::vector<MOTOR> on_commands;
            std::vector<MOTOR> off_commands;
            off_commands.push_back(M_LEFT_OFF);
            off_commands.push_back(M_RIGHT_OFF);
            on_commands.push_back(M_RIGHT_ON);
            on_commands.push_back(M_LEFT_ON);
            if(strcmp(argv[3], "1") == 0){
                b->apply_motor_commands(on_commands);
            }
            else if(strcmp(argv[3], "0") == 0){
                b->apply_motor_commands(off_commands);
            }
        }
        else{
            test_network();
        }
    }
    else if(argc > 1 && strcmp(argv[1], "calibrate") == 0){
        system("gtimeout 2 ./mdns-mod -B");
        int device_index = 0;
        if(argc > 2){
            device_index = atoi(argv[2]);
        }
        main_loop(device_index, CONTROL);
    }
    else if(argc > 1 && strcmp(argv[1], "watch") == 0) {
        system("gtimeout 2 ./mdns-mod -B");
        int device_index = 0;
        if (argc > 2) {
            device_index = atoi(argv[2]);
        }
        main_loop(device_index, WATCH);
    }
    else{
        calibrate_camera_main(argc, argv);
    }
    return 0;
}

