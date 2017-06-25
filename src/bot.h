//
// Created by Sam Royston on 12/18/16.
//

#ifndef CONTROL_BOT_H
#define CONTROL_BOT_H

#include <opencv2/opencv.hpp>
#include <deque>
#include "agent.h"

enum MOTOR{
    M_RIGHT_OFF = 0,
    M_LEFT_OFF = 1,
    M_RIGHT_ON = 2,
    M_LEFT_ON = 3
};

enum COMMAND{
    C_BOTH_OFF = 0,
    C_LEFT_OFF_RIGHT_ON = 1,
    C_LEFT_ON_RIGHT_OFF = 2,
    C_BOTH_ON = 3
};

struct phys_state{
    cv::Vec3d location;
    cv::Vec3d velocity;
    cv::Point2d rotation;
};

class Bot {
public:
    Bot( const char *host );
    ~Bot();

    StateAction Q_indices(cv::Point2f location, cv::Point2f rotation);
    void save_to_file();
    void load_info_image();
    void set_color(std::vector<StateAction>);
    void set_color(std::vector<StateAction>, cv::Vec3f);
    void apply_motor_commands( std::vector<MOTOR> );
    void record_motor_commands( std::vector<MOTOR> );
    void set_target_location( cv::Point );
    void incr_command_queue();
    void learn();
    void update_image_for_state(StateAction prev, StateAction s);
    static void match_movement(int, int);
    StateAction train_action(StateAction state_action);
    phys_state state;
    StateAction current_state;
    cv::Point2d target;
    std::vector<cv::Mat> info_images;
    float max_distance;
    char *host;
    std::deque< std::vector<MOTOR> > command_queue;
    bool on_off_inverted;
    bool lr_inverted;
    int aruco_id;
    bool training;
    int action_memory;
    Agent *agent;
private:
    float left_motor_on;
    int port;
    float right_motor_on;
    std::deque<std::pair<StateAction, long int> > past_state_actions;

};


#endif //CONTROL_BOT_H
