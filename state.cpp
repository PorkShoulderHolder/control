//
// Created by Sam Royston on 12/19/16.
//

#include <opencv2/videoio.hpp>
#include "state.h"

using namespace std::chrono;

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


void State::update(cv::Mat image) {
    State::difference_image = State::current_image - image;

    State::current_image = image;
    if(State::image_queue.size() >= State::hist_length){
    	State::image_queue.pop_back();
    }

    unsigned long ms = duration_cast< milliseconds >
	    (system_clock::now().time_since_epoch()).count();
    t_frame current;
    current.image = image;
    current.ms = ms;
    State::image_queue.push_front(current);
}

