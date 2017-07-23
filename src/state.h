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

void init_state();

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
public:

    std::vector<Bot> bots_for_ids(std::vector<char *> hosts, std::vector<int> ids);
    static void update(cv::Mat image);
    static void schedule_task(task t, int frame_offset);
    static int hist_length;
    static std::vector<Bot*> devices;
	static std::set<int> marker_ids;
    static Behavior *behavior;
    static cv::Mat current_image;
    static cv::Mat difference_image;
    static std::deque<t_frame> hist;
    static std::deque< frame_tasks > task_queue;
    static cv::Mat display_image;
    static cv::Mat info_image;
private:
    static std::pair<std::unordered_map<int, cv::Vec3d>, std::unordered_map<int, cv::Vec3d> >
            update_markers(cv::Mat);

    static cv::aruco::Dictionary marker_dictionary;
};


#endif //CONTROL_STATE_H
