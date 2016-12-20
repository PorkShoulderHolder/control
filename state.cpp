//
// Created by Sam Royston on 12/19/16.
//

#include <opencv2/videoio.hpp>
#include "state.h"

using namespace std::chrono;

std::vector<std::pair<char *, int> > State::match_mdns_aruco(std::vector<char *> hosts, std::vector<int> ids) {

}

std::vector<Bot> State::bots_for_ids(std::vector<char *> hosts, std::vector<int> ids) {
}


void State::update(cv::Mat image) {
    State::difference_image = State::current_image - image;
    State::image_queue.push(image);
    State::current_image = image;
    milliseconds ms = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch()
    );
}


cv::Mat current_image = cv::Mat::zeros(10, 10, CV_32F);
