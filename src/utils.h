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
#include "state.h"

typedef std::pair<std::vector<cv::Vec3d>, std::vector<cv::Point2d> > LocationRotationVec;
typedef std::pair<std::unordered_map<int, cv::Vec3d>, std::unordered_map<int, cv::Vec3d> > LocationRotationMap;

std::vector<MOTOR> motor_instructions(COMMAND command_type);
COMMAND command_code(std::vector<MOTOR> command);

class Utils{
public:
    static cv::Point2d getRotationFromQuad(std::vector<cv::Point2f> quad );
    static LocationRotationVec compute_ground_plane(std::vector< std::vector<cv::Point2f> > quads);
    static cv::Mat homogenous_quad( std::vector<cv::Point2f> quad);
    static bool begin_match_aruco();
    static std::pair<int, int> get_lr(char* bot_name);
    static std::vector<char *> get_device_names_from_file();
    static cv::Point2f find_closest(std::vector<Bot *> bots, Bot *bot);
    static cv::Point2f centroid(std::vector<Bot *> bots);
private:

    static cv::Mat distortion_coefs;
    static cv::Mat camera_matrix;
};

#endif //CONTROL_UTILS_H
