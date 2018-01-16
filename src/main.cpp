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

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#define DEBUG false
#define CONTROL 0
#define WATCH 1
#define PREDATOR_PREY 2
#define FOLLOW 3

#define RIGHT_ARROW_KEY 63235
#define LEFT_ARROW_KEY 63234
#define TOP_ARROW_KEY 63232
#define BOT_PREFIX "goodvibes"

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
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_1000);
    int i;

    cv::Mat display_image = cv::Mat::ones((count / 5) * 150 + 20, 760, CV_32F);
    for(i = 0 ;i < count; i++ ){

      cv::aruco::drawMarker(dictionary, i, 100, markerImage, 1);
      int x = (i % 5) * 150 + 10;
      int y = (i / 5) * 150 + 20;
      std::string s = std::to_string(i);
      cv::Point p;
      p.x = x + 22;
      p.y = y + 3;
      cv::Rect r = cv::Rect(x + 20, y + 20, 100, 100);
      markerImage.copyTo(display_image(r));
      cv::putText(display_image, s, p, 1, 1, cv::Scalar(0, 0, 0), 1); 
    }
    cv::imshow(MAIN_WINDOW, display_image);
    cv::waitKey(10000);
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
    cv::Mat obstacles;
    /* event loop */
    while(1){
        S = State::shared_instance();
        input_stream >> current_image;
        i++;
        cv::waitKey(10);
        if(i > 20){
            float ratio = (float)current_image.rows / (float)current_image.cols;
            float s = 185.0f;
            cv::Mat local;
            cv::resize(current_image, local, cv::Size((int)(s / ratio), (int)s));
            cv::cvtColor(local, local, CV_RGB2GRAY);
            local.convertTo( S->info_image, CV_32FC1);


            std::vector<KeyPoint> keypoints;
            std::vector<cv::Mat> contours;

            cv::GaussianBlur(local, local,Size(3,3), 0, 0, BORDER_DEFAULT );

            cv::adaptiveThreshold(local, S->info_image, 255, ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY,19, 5);

            cv::bitwise_not(S->info_image, S->info_image);
            Mat erode_element = getStructuringElement( MORPH_ELLIPSE, Size( 1, 1 ));
            Mat dilate_element = getStructuringElement( MORPH_ELLIPSE, Size( 4, 4 ));
            cv::erode(S->info_image, S->info_image, erode_element);
            cv::dilate(S->info_image, S->info_image, dilate_element);
            list<state> path;

            if(obstacles.size().height == 0  && S->devices.size() > 0){

                Bot *bot = S->devices.back();

                bot->start_pathfinding(S->info_image, bot->target);

                path = bot->pathfinder->getPath();
                obstacles = S->info_image;

            }
            else if(S->devices.size() > 0){
                Bot *bot = S->devices.back();
                cv::Mat diff = obstacles - S->info_image;
                bot->update_pathfinding(diff);
                path = bot->pathfinder->getPath();
                std::cout << path.size() << std::endl;

                for(state ts : path){
                    circle(S->info_image, cv::Point2d(ts.x, ts.y),
                           2.0,
                           Scalar( 0, 0, 255 ),
                           1.0,
                           8 );
                }
                obstacles = S->info_image;

            }





            cv::imshow(INFO_WINDOW, S->info_image);

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
            if(i == -1) break;
            }
    }
}

void test_network(){
    auto *manager = new NetworkManager();
    manager->send_udp((char *) "goodvibes1067796.local", (char *) "r:::1", 8888);
}
#ifdef __linux__

string get_stdout(string cmd) {
    string data;
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

    stream = popen(cmd.c_str(), "r");
    if (stream) {
        while (!feof(stream))
            if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
        pclose(stream);
    }
    return data;
}

#endif

int main(int argc, const char** argv )
{
    std::map<std::string, int> config;
    config["follow"] = FOLLOW;
    config["hunt"] = PREDATOR_PREY;

    ArgumentParser parser;
    parser.addArgument("-r", "--run", '*');
//    parser.addArgument("-w", "--watch");
//    parser.addArgument("-c", "--camera");
//    parser.addArgument("-g", "--generate-markers");

    parser.parse((size_t)argc, argv);

    std::signal(SIGINT, exiting);
    srand (static_cast <unsigned> (time(0)));
    if (parser.exists("generate-markers")) {
        int count = 50;
        if (argc > 2) {
            count = atoi(argv[2]);
        }
        init_application();
        generate_markers(count);
    }
    else
    if(strcmp(argv[1], "calibrate_camera") == 0){
        calibrate_camera_main(argc, argv);
    }
    else{

#ifdef __APPLE__
        system("gtimeout 2 ./mdns-mod -B");
#elif __linux__
        std::string out = get_stdout("timeout 2 mdns-scan");
        std::cout << out << std::endl;
        std::replace(out.begin(), out.end(), '\n', ' ');
        std::stringstream iss(out);
        vector<string> s_array;
        string temp;
        while (iss >> temp)
            s_array.push_back(temp); //
        for (std::string line : s_array)
        {
            string linecpy = line;
            unsigned long loc = line.find(BOT_PREFIX);
            if(loc != std::string::npos){
                unsigned long i = linecpy.find('.');
                std::string device_name = line.substr(0, i);
                FILE *f = fopen("domains.txt", "w+");
                fprintf(f, "%s.local\n", device_name.c_str());
                fclose(f);
            }
        }

#endif
        int device_index = 0;


        if(parser.exists("camera")){
            device_index = parser.retrieve<int>("camera");
        }
        if(parser.exists("run")){
            int sub_mode = 0;
            vector<string> arg = parser.retrieve<vector<string>>("run");
            std::string a = arg[0];
            int device = device_index;
            if(arg.size() > 1){
                device = atoi(arg[1].c_str());
            }
            if(config.count(a) > 0){
                sub_mode = config[a];
            }
            else{
                std::cout << "unrecognized arg '" + a + "'" << std::endl;
            }
            main_loop(device, CONTROL, sub_mode);
        }
        else if(parser.exists("watch")) {
            main_loop(device_index, WATCH, 0);
        }
    }
    return 0;
}

