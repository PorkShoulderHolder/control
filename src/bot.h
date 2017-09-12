//
// Created by Sam Royston on 12/18/16.
//

#ifndef CONTROL_BOT_H
#define CONTROL_BOT_H
#define BOT_SETTINGS_FILE "config.txt"
#include <opencv2/opencv.hpp>
#include <deque>
#include "agent.h"
#include "d_star.h"


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

typedef state dstar_state;

class Bot {
public:
    Bot( const char *host );
    ~Bot();

    StateAction Q_indices(cv::Point2f location, cv::Point2f rotation);
    void apply_motor_commands( std::vector<MOTOR> );
    void incr_command_queue();
    void act();
    static void match_movement(int, int);
    StateAction control_action(StateAction state_action);
    phys_state state;
    StateAction current_state;
    cv::Point2d target;
    std::vector<cv::Mat> info_images;
    char *host;
    std::deque< std::vector<MOTOR> > command_queue;
    std::deque< cv::Point2d > target_queue;
    void append_path(std::vector< cv::Point2d >);
    void replace_path(std::vector< cv::Point2d >);
    int left_command;
    int right_command;
    int aruco_id;
    std::string to_string();
    bool training;
    bool evasive;
    int action_memory;
    Agent *agent;
    void initialize_pathfinder(cv::Mat &m);
    std::list<dstar_state> update_paths(cv::Mat &od, cv::Mat &ob);
    int port;
private:
    std::string send_state(StateAction sa);
    float reached_target_thresh;
    std::deque<std::pair<StateAction, long int> > past_state_actions;
    Dstar *pathfinder;
};




#endif //CONTROL_BOT_H
