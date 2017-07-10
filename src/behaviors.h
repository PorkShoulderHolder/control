//
// Created by Sam Royston on 7/8/17.
//

#ifndef CONTROL_BEHAVIORS_H
#define CONTROL_BEHAVIORS_H

#import <bot.h>
#include <vector>
#include "bot.h"
#include "utils.h"


class Behavior {
public:
    Behavior();
    Behavior(std::vector<Bot*> bots);
    virtual void update();
    std::vector<Bot*> bots;
private:
};

class FollowTheLeader : Behavior{
    FollowTheLeader();
    FollowTheLeader(cv::Point2f t);
    void update();
    cv::Point2f target;
};

class PredatorPrey : Behavior{
public:
    PredatorPrey();
    PredatorPrey(std::vector<Bot *> bots);
    PredatorPrey(std::vector<Bot*> predators, std::vector<Bot*> prey);
    void update();
    int mode;
    cv::Point2f target;
    std::vector<Bot*> predators;
    std::vector<Bot*> prey;
private:
    void avoid_nearest();
    void avoid_centroid();
};


#endif //CONTROL_BEHAVIORS_H
