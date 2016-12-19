//
// Created by Sam Royston on 12/18/16.
//

#ifndef CONTROL_BOT_H
#define CONTROL_BOT_H

#include <opencv2/core/types.hpp>

enum MOTOR{ M_RIGHT_OFF, M_LEFT_OFF, M_RIGHT_ON, M_LEFT_ON };


class Bot {
public:
    Bot(const char *host);
    cv::Point location;
    cv::Point velocity;
    float orientation;
    char *host;
    int aruco_id;
    void set_target_location(cv::Point);
    void apply_motor_commands(const MOTOR commands[2]);
private:
    float left_motor_on;
    int port;
    float right_motor_on;

};


#endif //CONTROL_BOT_H
