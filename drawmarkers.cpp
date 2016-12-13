#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp> 
#include <string> 

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
    waitKey(0);
  
}

void calibrate(int device_index){
    cv::VideoCapture input_stream(0);
    cv::Mat current_image;
    while(1){
        input_stream.read(current_image);



        cv::Mat inputImage = current_image;  
        vector< int > markerIds; 
        vector< vector<Point2f> > markerCorners, rejectedCandidates; 
        cv::aruco::DetectorParameters parameters; 
        cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250); 
        cv::aruco::detectMarkers(inputImage, dictionary, markerCorners, markerIds, parameters, rejectedCandidates); 
        cv::aruco::drawDetectedMarkers(current_image, markerCorners, markerIds); 
        cv::imshow("output", current_image);
        if(cv::waitKey(30) >= 0) break;
    }
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
    else if(argc > 1 && strcmp(argv[1], "calibrate") == 0){
        int device_index = 1;
        if(argc > 2){
            device_index = atoi(argv[2]);
        }
        calibrate(device_index);
    }
    return 0;
}

