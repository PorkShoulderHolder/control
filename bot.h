//
// Created by Sam Royston on 12/18/16.
//

#ifndef CONTROL_BOT_H
#define CONTROL_BOT_H

#include <opencv2/core/types.hpp>
#include <deque>

enum MOTOR{
    M_RIGHT_OFF = 0,
    M_LEFT_OFF = 1,
    M_RIGHT_ON = 2,
    M_LEFT_ON = 3
};


class Bot {
public:
    Bot(const char *host);
    cv::Vec3d location;
    cv::Vec3d velocity;
    cv::Vec3d rotation;
    char *host;
    std::deque<std::vector<MOTOR> > command_queue;
    int aruco_id;
    void set_target_location(cv::Point);
    void apply_motor_commands(std::vector<MOTOR>);
    void incr_command_queue();
private:
    float left_motor_on;
    int port;
    float right_motor_on;
};


#endif //CONTROL_BOT_H
