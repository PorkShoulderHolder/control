//
// Created by Sam Royston on 12/19/16.
//

#ifndef CONTROL_UTILS_H
#define CONTROL_UTILS_H

#include <stdlib.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include "bot.h"

class Utils{
public:
    static std::pair<std::vector<cv::Vec3d>, std::vector<cv::Vec3d> >
            compute_ground_plane(std::vector< std::vector<cv::Point2f> > quads);
    static cv::Mat homogenous_quad( std::vector<cv::Point2f> quad);
    static bool begin_match_aruco();
    static std::vector<char *> get_device_names_from_file();

private:

    static cv::Mat distortion_coefs;
    static cv::Mat camera_matrix;
};

#endif //CONTROL_UTILS_H
