#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp> 
#include <string>
#include "network_manager.h"
#include "bot.h"
#include "state.h"

using namespace cv;
using namespace std;

void generate_markers(int count){
    cv::Mat markerImage; 
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_1000);
    int i;
    int pw = 100;
    
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
      cv::Mat si = display_image(r);
      markerImage.copyTo(display_image(r));
      cv::putText(display_image, s, p, 1, 1, cv::Scalar(0, 0, 0), 1); 
    }
    cv::imshow("d", display_image);
    //waitKey(0);
  
}


void calibrate(int device_index){
    cv::VideoCapture input_stream(device_index);

    cv::Mat current_image;
    int i = 1;
    time_t start, finish;
    time(&start);
    std::string fps_str = "0 fps";
    int sample_pd = 80;
    cv::Point text_loc(20, 20);
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
//        for(int j = 0; j < State::image_queue.size(); j++){
//            t_frame t = State::image_queue[j];
//            for ( auto local_it = t.locations.begin(); local_it!= t.locations.end(); ++local_it ) {
//                std::cout << " " << local_it->first << ":" << local_it->second;
//            }
//            std::cout << "-----------" << std::endl;
//
//        }
//        std::cout << "=========" << std::endl;
        cv::putText(State::display_image, fps_str, text_loc, 1, 1, cv::Scalar(155, 155, 0), 1);
        cv::imshow("output", State::display_image);
        if(waitKey(30) >= 0) break;
        i++;
    }
}

void test_network(){
    NetworkManager *manager = new NetworkManager();
    manager->send_to((char *)"goodvibes1067796.local", (char *)"r:::1", 8888);
}

int main(int argc, char** argv )
{

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
            const MOTOR on_commands[2] = {M_RIGHT_ON, M_LEFT_ON};
            const MOTOR off_commands[2] = {M_RIGHT_OFF, M_LEFT_OFF};
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
        int device_index = 1;
        if(argc > 2){
            device_index = atoi(argv[2]);
        }
        calibrate(device_index);
    }
    return 0;
}

