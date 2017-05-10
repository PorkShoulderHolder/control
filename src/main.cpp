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
    printf("%c[2K", 27);
    std::cout << "\n --- bye :^) --- \n" << std::endl;
    std::exit(0);
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

void loop_outer_log(){
    for(int j = 0; j < State::hist.size(); j++){
        t_frame t = State::hist[j];
        for ( auto local_it = t.locations.begin(); local_it!= t.locations.end(); ++local_it ) {
            std::cout << " " << local_it->first << ":" << local_it->second;
        }
        std::cout << "-----------" << std::endl;

    }
    std::cout << "=========" << std::endl;
}
std::string type2str(int type) {
    std::string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch ( depth ) {
        case CV_8U:  r = "8U"; break;
        case CV_8S:  r = "8S"; break;
        case CV_16U: r = "16U"; break;
        case CV_16S: r = "16S"; break;
        case CV_32S: r = "32S"; break;
        case CV_32F: r = "32F"; break;
        case CV_64F: r = "64F"; break;
        default:     r = "User"; break;
    }

    r += "C";
    r += (chans+'0');

    return r;
}

void draw_bots_data(){
    int i = 0;
    int width = 200;
    int buffer = 10;
    int items_per_row = 5;
    int frame_widthx = 22;
    int frame_widthy = 3;
    cv::Mat display_image((int)(2 + (State::devices.size() / items_per_row)) * (width + 2 * buffer) + (2 * frame_widthy),
                          items_per_row * (width + buffer) + 2 * frame_widthx,
                          CV_32FC3);

    for(Bot *b: State::devices){

        int x = (i % items_per_row) * width + buffer;
        int y = (int)(i / items_per_row) * width + buffer * 2;
        std::string s = std::to_string(i);
        cv::Point p;
        p.x = x + frame_widthx;
        p.y = y + frame_widthy;
        cv::Rect r = cv::Rect(x + 2 * buffer, y + 2 * buffer, width, width);

        double sum = 0.0;
        int v = 2;
        for(int k = 0; k < b->info_images.size(); k++){
            cv::Mat m = b->info_images[k];
            cv::Rect sub_r(r.x + (k % v) * (width / 2), r.y + (k / v) * (width / 2), width / 2, width / 2);
            cv::Mat ma;
            double min, max;
            m.copyTo(ma);
            cv::resize(ma, ma, sub_r.size());
            ma = ma.t();
            ma.copyTo(display_image(sub_r));
        }
        cv::putText(display_image, s, p, 1, 1, cv::Scalar(0, 0, 0), 1);
        i++;
    }
    State::info_image = display_image;

}

void calibrate(int device_index){
    
    std::cout << "here " << std::endl;
    cv::VideoCapture input_stream(device_index);

    std::cout << "here " << std::endl;
    if (!input_stream.isOpened()) {
        std::cout << "Unable to read stream from specified device." << std::endl;
        return;
    }
    std::cout << "here " << std::endl;
    cv::Mat current_image;
    std::cout << "here " << std::endl;
    int i = 1;
    time_t start, finish;
    time(&start);
    std::string fps_str = "0 fps";
    int sample_pd = 80;
    cv::Point text_loc(20, 20);
    cv::namedWindow(MAIN_WINDOW, cv::WINDOW_NORMAL );
    cv::namedWindow(INFO_WINDOW, cv::WINDOW_NORMAL );

    init_state();


    while(1){

        if(i % sample_pd == 0){
            time(&finish);
            double seconds = difftime (finish, start);
            double fps = sample_pd / seconds;
            fps_str = std::string(to_string(fps)) + " fps";
            time(&start);
        }
        input_stream >> current_image;

        State::update(current_image);
        if(DEBUG){
            loop_outer_log();
        }

        cv::Mat final_draw;
        cv::resize(State::display_image, final_draw, Size(612, 384));

        cv::putText(final_draw, fps_str, text_loc, 1, 1, cv::Scalar(155, 155, 0), 1);
        cv::imshow(MAIN_WINDOW, final_draw);
        cv::imshow(INFO_WINDOW, State::info_image);
        draw_bots_data();

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
        calibrate(device_index);
    }
    else{
        calibrate_camera_main(argc, argv);
    }
    return 0;
}

