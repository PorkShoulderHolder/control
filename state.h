//
// Created by Sam Royston on 12/19/16.
//

#define DISPLAY_ON true

#include "bot.h"
#include <opencv2/core.hpp>
#include <deque>
#include <unordered_map>
#include <vector>
#include <set>
#include <stdlib.h>
#include "utils.h"

#ifndef CONTROL_STATE_H
#define CONTROL_STATE_H


/*
 * there is only meant to be one state per process
 */

struct t_frame{
	std::unordered_map<int, cv::Point2f> locations;
	long long int ms;
};

class State{
public:
    std::vector<std::pair<char *, int> > match_mdns_aruco(std::vector<char *> hosts, std::vector<int> ids);
    std::vector<Bot> bots_for_ids(std::vector<char *> hosts, std::vector<int> ids);
    static void update(cv::Mat image);
	static int hist_length;
    static std::vector<Bot> devices;
	static std::set<int> marker_ids;
    static cv::Mat current_image;
    static cv::Mat difference_image;
    static std::deque<t_frame> image_queue;
    static cv::Mat display_image;

private:
    static std::unordered_map<int, cv::Point2f> update_markers(cv::Mat);
    static cv::aruco::Dictionary marker_dictionary;
	static std::vector< std::unordered_map<int, cv::Point2f> > get_differentials();
};


#endif //CONTROL_STATE_H
