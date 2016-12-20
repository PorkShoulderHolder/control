//
// Created by Sam Royston on 12/19/16.
//
#include "bot.h"
#include <opencv2/core.hpp>
#include <queue>
#include <vector>
#include <stdlib.h>

#ifndef CONTROL_STATE_H
#define CONTROL_STATE_H


/*
 * there is only meant to be one state per process
 */


class State{
public:
    std::vector<std::pair<char *, int> > match_mdns_aruco(std::vector<char *> hosts, std::vector<int> ids);
    std::vector<Bot> bots_for_ids(std::vector<char *> hosts, std::vector<int> ids);
    void update(cv::Mat image);

    static std::vector<Bot> devices;
    static cv::Mat current_image;
    static cv::Mat difference_image;
    static std::queue<cv::Mat> image_queue;
};


std::vector<Bot> State::devices;
std::queue<cv::Mat> State::image_queue;
cv::Mat State::current_image;
cv::Mat State::difference_image;


#endif //CONTROL_STATE_H
