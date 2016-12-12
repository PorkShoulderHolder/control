#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp> 

using namespace cv;

int main(int argc, char** argv )
{
    
    int count = 25;
    if (argc > 1){
      count = atoi(argv[1]); 
    }
    cv::Mat markerImage; 
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250);
    int i;
    int pw = 100;
    
    int width = sqrt(count);
    cv::Mat display_image = cv::Mat::zeros(pw * width, pw * width, CV_32F);

    for(i = 0 ;i < count; i++ ){

      cv::aruco::drawMarker(dictionary, i, width, markerImage, 1);
      int x = i % width;
      int y = i / width;
      markerImage.copyTo(display_image(cv::Rect(x * pw , y * pw, pw, pw)));
    }
    cv::imshow("d", display_image);
    waitKey(0);

    return 0;
}
