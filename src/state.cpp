//
// Created by Sam Royston on 12/19/16.
//


#include <opencv2/videoio.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include "state.h"
#include "utils.h"


using namespace std::chrono;
using namespace cv;
using namespace std;


typedef std::pair<std::vector<cv::Vec3d>, std::vector<cv::Point2d> > LocationRotationVec;
typedef std::pair<std::unordered_map<int, cv::Vec3d>, std::unordered_map<int, cv::Vec3d> > LocationRotationMap;

std::vector<Bot> State::bots_for_ids(std::vector<char *> hosts, std::vector<int> ids) {
    //TODO: finish
    std::vector<Bot> ds;
    return ds;
}


LocationRotationMap State::update_markers(cv::Mat image){
    vector< int > ids;
    vector< vector<Point2f> > marker_corners, rejected;
    cv::aruco::DetectorParameters parameters;
    cv::aruco::detectMarkers(image, State::marker_dictionary, marker_corners, ids, parameters, rejected);
    std::pair<std::vector<cv::Vec3d>, std::vector<cv::Point2d> > normal_pos = Utils::compute_ground_plane(marker_corners);
    std::vector<cv::Vec3d> locations = normal_pos.first;
    std::vector<cv::Point2d> rotations = normal_pos.second;

    std::unordered_map<int, cv::Vec3d> marker_locations;
    std::unordered_map<int, cv::Vec3d> marker_rotations;

    State::marker_ids.insert(ids.begin(), ids.end());
    cv::Vec3d target(0,0,0);

    for(int i = 0; i < ids.size(); i++){

        cv::Vec3d location = locations[i];
        cv::Vec3d rotation(rotations[i].x, rotations[i].y, 0);
        int id = ids[i];

        if(id != TARGET_MARKER){
            std::pair<int, cv::Vec3d> loc_kv(id, location);
            std::pair<int, cv::Vec3d> rot_kv(id, rotation);
            marker_locations.insert(loc_kv);
            marker_rotations.insert(rot_kv);
        }
        else{
            target = location;
        }
    }

    for( Bot *b : State::devices ){
        b->state.location = marker_locations[b->aruco_id];
        b->state.rotation = cv::Vec2d(marker_rotations[b->aruco_id][0], marker_rotations[b->aruco_id][1]);
        b->target = cv::Point(target[0], target[1]);
    }

    if(DISPLAY_ON){
        cv::aruco::drawDetectedMarkers(State::display_image, marker_corners, ids);
    }
    LocationRotationMap p(marker_locations, marker_rotations);
    return p;
}

void init_state(){
    std::vector<char *> hosts = Utils::get_device_names_from_file();

    for (char *host : hosts){

        Bot *b  = new Bot(host);
        State::devices.push_back(b);
    }

    Utils::begin_match_aruco();
}

void State::schedule_task(task t, int frame_offset) {
    while( frame_offset >= State::task_queue.size() ){
        frame_tasks empty_tasks;

        State::task_queue.push_back(empty_tasks);
        if(State::task_queue.size() >= MAX_TASK_DEQUE_SIZE){
            std::cout << "MAX_TASK_DEQUE_SIZE reached, task not scheduled" << std::endl;
            return;
        }
    }
    State::task_queue[frame_offset - 1].scheduled_tasks.push_back(t);
}

void State::update(cv::Mat image) {

    State::display_image = image;
    State::current_image = image;
    if(State::hist.size() >= State::hist_length){
    	State::hist.pop_back();
    }

    long long int ms = duration_cast< milliseconds >
	    (system_clock::now().time_since_epoch()).count();
    t_frame current;
    current.ms = ms;
    LocationRotationMap cur_device_states = State::update_markers(image);
    current.locations = cur_device_states.first;

    State::hist.push_front(current);

    for( Bot *b : State::devices ){
        b->incr_command_queue();
    }
    if(State::task_queue.size() > 0){
        State::task_queue.front()();
        State::task_queue.pop_front();
    }
}

int State::hist_length = 20;
std::vector< Bot* > State::devices;
std::deque<t_frame> State::hist;
std::deque< frame_tasks > State::task_queue;
cv::Mat State::current_image;
cv::Mat State::difference_image;
cv::Mat State::display_image;
cv::Mat State::info_image = cv::Mat::zeros(10,10, CV_32F);

std::set< int > State::marker_ids;
cv::aruco::Dictionary State::marker_dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

