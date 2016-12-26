//
// Created by Sam Royston on 12/19/16.
//

#ifndef CONTROL_UTILS_H
#define CONTROL_UTILS_H

#include <stdlib.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

class Utils{
    static std::vector<cv::Point2f> compute_ground_plane(std::vector< std::vector<cv::Point2f> > quads);
    static cv::Mat homogenous_quad( std::vector<cv::Point2f> quad);
};

#endif //CONTROL_UTILS_H
