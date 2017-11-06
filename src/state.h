//
// Created by Sam Royston on 12/19/16.
//

#ifndef CONTROL_STATE_H
#define CONTROL_STATE_H

#define INFO_WINDOW "Info"
#define MAIN_WINDOW "Display"
#define TARGET_MARKER 30

#define DISPLAY_ON true
#define MAX_TASK_DEQUE_SIZE 1000
#include "bot.h"
#include <opencv2/core.hpp>
#include <deque>
#include <unordered_map>
#include <vector>
#include <set>
#include <stdlib.h>
#include "utils.h"
#include "behavior_base.h"
#import "behaviors.h"


/*
 * there is only meant to be one state per process
 */

struct t_frame{
    std::unordered_map<int, cv::Vec3d> locations;
    std::unordered_map<int, cv::Point2d> rotations;
    long long int ms;
};

struct task {
    int id;
    int duration;
    std::function<void(int, int)> f;
};

struct frame_tasks {
    std::vector<task> scheduled_tasks;
    void operator()() {
        for(task t : scheduled_tasks){
            t.f(t.id, t.duration);
        }
    }
};

class State{
    /*
     *  - devices: vector of Bot objects, assume nothing about the order
     *  - hist: a deque of length State::hist_length of past device states,
     *    useful for time based analysis of device movement, the first element
     *    is the most recent (current) locations, and the last is the furthest
     *    back in time
     *  - static void schedule_task(task t, int frame_offset): allows us to set up a future task
     *    to be executed frame_offset frames in the future
     */
    State(){};
    State(State const&){};             // copy constructor is private
    State& operator=(State const&){};
    static State *_instance;
public:
    std::vector<Bot> bots_for_ids(std::vector<char *> hosts, std::vector<int> ids);
    void update(cv::Mat image);
    void schedule_task(task t, int frame_offset);
    int hist_length;
    std::vector<Bot*> devices;
	std::set<int> marker_ids;
    Behavior *behavior;
    cv::Mat current_image;
    cv::Mat difference_image;
    std::deque<t_frame> hist;
    std::deque< frame_tasks > task_queue;
    cv::Mat display_image;
    cv::Point2f target;
    cv::Mat info_image;

    static State *shared_instance();

private:
    std::pair<std::unordered_map<int, cv::Vec3d>, std::unordered_map<int, cv::Vec3d> >
            update_markers(cv::Mat);

    cv::Ptr<cv::aruco::Dictionary> marker_dictionary;
};


#endif //CONTROL_STATE_H
