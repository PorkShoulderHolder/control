//
// Created by Sam Royston on 12/19/16.
//


#include <opencv2/videoio.hpp>
#include <opencv2/aruco.hpp>
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



std::unordered_map<int, Point2f> State::update_markers(cv::Mat image){
    vector< int > ids;
    vector< vector<Point2f> > marker_corners, rejected;
    cv::aruco::DetectorParameters parameters;
    cv::aruco::detectMarkers(image, State::marker_dictionary, marker_corners, ids, parameters, rejected);
    t_frame current;
    std::unordered_map<int, Point2f> marker_locations;
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

int State::hist_length = 40;
std::vector<Bot> State::devices;
std::deque<t_frame>State::image_queue;
cv::Mat State::current_image;
cv::Mat State::difference_image;
cv::Mat State::display_image;
cv::aruco::Dictionary State::marker_dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250);

