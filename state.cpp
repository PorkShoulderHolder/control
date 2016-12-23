//
// Created by Sam Royston on 12/19/16.
//


#include <opencv2/videoio.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include "state.h"


using namespace std::chrono;
using namespace cv;
using namespace std;

std::vector<std::pair<char *, int> > State::match_mdns_aruco(std::vector<char *> hosts, std::vector<int> ids) {
    //TODO: finish
    std::vector<std::pair<char *, int> > op;
    return op;
}

std::vector<Bot> State::bots_for_ids(std::vector<char *> hosts, std::vector<int> ids) {
    //TODO: finish
    std::vector<Bot> ds;
    return ds;
}


std::vector< std::unordered_map<int, cv::Point2f> > State::get_differentials() {
    float mvmt_threshold = 3.0;
    for(int id : State::marker_ids) {
        for(int i = 1; i < (i <  State::image_queue.size() ? 2 : State::image_queue.size()); i++){
            t_frame frame = State::image_queue[i];
            t_frame prev = State::image_queue[i - 1];
            auto current_loc = frame.locations.find(id);
            auto prev_loc = prev.locations.find(id);
            if(current_loc != frame.locations.end() && prev_loc != prev.locations.end()){
                double d = cv::norm(current_loc->second - prev_loc->second);
                if(d > mvmt_threshold){
                    std::cout << "marker " << id << " moving" << std::endl;
                    break;
                }
            }
        }
    }
};

std::unordered_map<int, Point2f> State::update_markers(cv::Mat image){
    vector< int > ids;
    vector< vector<Point2f> > marker_corners, rejected;
    cv::aruco::DetectorParameters parameters;
    cv::aruco::detectMarkers(image, State::marker_dictionary, marker_corners, ids, parameters, rejected);
    std::unordered_map<int, Point2f> marker_locations;

    State::marker_ids.insert(ids.begin(), ids.end());
    for(int i = 0; i < marker_corners.size(); i++){
        vector<Point2f> box = marker_corners[i];
        int id = ids[i];
        Point2f avg;

        for(Point2f p : box){
            avg.x += p.x / box.size();
            avg.y += p.y / box.size();
        }
        std::pair<int, Point2f> kv(id, avg);
        marker_locations.insert(kv);

    }
    State::get_differentials();

    if(DISPLAY_ON){
        cv::aruco::drawDetectedMarkers(State::display_image, marker_corners, ids);
    }
    return marker_locations;
}


void State::update(cv::Mat image) {
//    State::difference_image = State::current_image - image;
    State::display_image = image;
    State::current_image = image;
    if(State::image_queue.size() >= State::hist_length){
    	State::image_queue.pop_back();
    }

    long long int ms = duration_cast< milliseconds >
	    (system_clock::now().time_since_epoch()).count();
    t_frame current;
    current.ms = ms;
    current.locations = State::update_markers(image);

    State::image_queue.push_front(current);
}

int State::hist_length = 20;
std::vector<Bot> State::devices;
std::deque<t_frame>State::image_queue;
cv::Mat State::current_image;
cv::Mat State::difference_image;
cv::Mat State::display_image;
std::set< int > State::marker_ids;
cv::aruco::Dictionary State::marker_dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250);

