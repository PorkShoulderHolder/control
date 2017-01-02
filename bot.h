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

struct phys_state{
    cv::Vec3d location;
    cv::Vec3d velocity;
    cv::Vec3d rotation;
};

class Bot {
public:
    Bot( const char *host );
    void save_to_file();
    void apply_motor_commands( std::vector<MOTOR> );
    void set_target_location( cv::Point );
    void incr_command_queue();
    static void match_movement(int);

    phys_state state;
    char *host;
    std::deque< std::vector<MOTOR> > command_queue;
    bool on_off_inverted;
    bool lr_inverted;
    int aruco_id;
private:
    float left_motor_on;
    int port;
    float right_motor_on;
};


#endif //CONTROL_BOT_H
