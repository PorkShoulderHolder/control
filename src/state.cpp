//
// Created by Sam Royston on 12/19/16.
//


#include <opencv2/videoio.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include "state.h"
#include "utils.h"
#import "behaviors.h"

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


void State::update_obstacle_bitmap(){
    float ratio = (float)this->current_image.rows / (float)this->current_image.cols;
    cv::Mat mat;

    cv::resize(this->current_image, mat, cv::Size((int)(45.0f / ratio), 45));
    cv::cvtColor(mat, mat, CV_RGB2GRAY);
    mat.convertTo(mat, CV_32FC1);
    cv::GaussianBlur( mat, mat,Size(3,3), 0, 0, BORDER_DEFAULT );
    cv::Laplacian( mat, mat, CV_8U, 3, 1, 0, BORDER_DEFAULT );
    cv::threshold(mat, mat, 15.0f, 255, CV_THRESH_BINARY);
    cv::accumulateWeighted(mat, this->obstacle_avg, 0.1);
    cv::threshold(this->obstacle_avg, mat, 15.0f, 255, CV_THRESH_BINARY);

    cv::Mat erode_element = getStructuringElement( MORPH_ELLIPSE, Size( 2, 2 ));
    cv::Mat dilate_element = getStructuringElement( MORPH_ELLIPSE, Size( 6, 6 ));

    cv::erode(mat, mat, erode_element);
    cv::dilate(mat, mat, dilate_element);
    if(this->obstacle_bitmap.rows > 0){
         cv::findNonZero(this->obstacle_bitmap - mat, this->obstacle_difference);
    }
    this->obstacle_bitmap = mat;
}

std::list<state> State::update_paths(){
    for(int i = 0; i < this->obstacle_difference.total(); i++){
        int x = this->obstacle_difference.at<cv::Point>(i).x;
        int y = this->obstacle_difference.at<cv::Point>(i).y;
        float value = this->obstacle_bitmap.at(x, y);
        this->pathfinder->updateCell(x, y, value);
    }
    return this->pathfinder->getPath();
}

void State::initialize_pathfinder(){
    int sx = this->obstacle_bitmap.cols;
    int sy = this->obstacle_bitmap.rows;

    // TODO: scale these coordinates correctly => this is WRONG!
    int tx = (int)this->target.x;
    int ty = (int)this->target.y;
    this->pathfinder->init(sx, sy, tx, ty);
    for(int x = 0; x < this->obstacle_bitmap.cols; x++){
        for (int y = 0; y < this->obstacle_bitmap.rows; ++y) {
            float val = this->obstacle_bitmap.at(x, y);
            this->pathfinder->updateCell(x, y, val);
        }
    }

}



LocationRotationMap State::update_markers(cv::Mat image){
    vector< int > ids;
    vector< vector<Point2f> > marker_corners, rejected;
    cv::aruco::DetectorParameters parameters;
    cv::aruco::detectMarkers(image, this->marker_dictionary, marker_corners, ids, parameters, rejected);
    std::pair<std::vector<cv::Vec3d>, std::vector<cv::Point2d> > normal_pos = Utils::compute_ground_plane(marker_corners);
    std::vector<cv::Vec3d> locations = normal_pos.first;
    std::vector<cv::Point2d> rotations = normal_pos.second;

    std::unordered_map<int, cv::Vec3d> marker_locations;
    std::unordered_map<int, cv::Vec3d> marker_rotations;

    this->marker_ids.insert(ids.begin(), ids.end());
    static cv::Vec3d target(0,0,0);

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
            this->target = cv::Point2d(target[0], target[1]);
        }
    }

    for( Bot *b : this->devices ){
        b->state.location = marker_locations[b->aruco_id];
        b->state.rotation = cv::Vec2d(marker_rotations[b->aruco_id][0], marker_rotations[b->aruco_id][1]);
//        b->target = cv::Point2d(target[0], target[1]);
    }

    if(DISPLAY_ON){
        cv::aruco::drawDetectedMarkers(this->display_image, marker_corners, ids);
    }
    LocationRotationMap p(marker_locations, marker_rotations);
    return p;
}

void State::schedule_task(task t, int frame_offset) {
    while( frame_offset >= this->task_queue.size() ){
        frame_tasks empty_tasks;

        this->task_queue.push_back(empty_tasks);
        if(this->task_queue.size() >= MAX_TASK_DEQUE_SIZE){
            std::cout << "MAX_TASK_DEQUE_SIZE reached, task not scheduled" << std::endl;
            return;
        }
    }
    this->task_queue[frame_offset - 1].scheduled_tasks.push_back(t);
}

void State::update(cv::Mat image) {
    this->display_image = image;
    this->current_image = image;
    if(this->hist.size() >= this->hist_length){
        this->hist.pop_back();
    }

    long long int ms = duration_cast< milliseconds >
	    (system_clock::now().time_since_epoch()).count();
    t_frame current;
    current.ms = ms;
    LocationRotationMap cur_device_states = this->update_markers(image);
    this->behavior->update();

    current.locations = cur_device_states.first;

    this->hist.push_front(current);

    for( Bot *b : this->devices ){
        b->incr_command_queue();
        if(b->training){
            this->obstacle_bitmap = cv::Mat::zeros(200,200, CV_8UC3);
            cv::Point2d p(2 * (-1 * b->current_state.x + 50), 2 * (-1 * b->current_state.y + 50));
            cv::Point2d r(100, 100);
            cv::circle(this->obstacle_bitmap, p, 10, cv::Scalar(244,244,0), 5);
            cv::line(this->obstacle_bitmap, p, r, cv::Scalar(0,244,244), 5);
        }
    }

    if(this->task_queue.size() > 0){
        this->task_queue.front()();
        this->task_queue.pop_front();
    }

}


State *State::shared_instance(){
    if(!_instance){
        _instance = new State;
        _instance->hist_length = 20;
        _instance->info_image = cv::Mat::zeros(200,200, CV_8UC3);
        _instance->marker_dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
        std::vector<char *> hosts = Utils::get_device_names_from_file();
        std::cout << hosts.size() << "djdjdjdj" << std::endl;
        for (char *host : hosts){
            Bot *b  = new Bot(host);
            _instance->devices.push_back(b);
        }
        Utils::begin_match_aruco();
        _instance->pathfinder = new Dstar();
    }
    return _instance;
};

State *State::_instance = NULL;
