//
// Created by Sam Royston on 7/8/17.
//

#ifndef CONTROL_BEHAVIORS_H
#define CONTROL_BEHAVIORS_H

#import "bot.h"
#include "utils.h"
#include "behavior_base.h"
#include <vector>




class FollowTheLeader : public Behavior{
public:
    FollowTheLeader();
    FollowTheLeader(cv::Point2f t);
    virtual void update();
    cv::Point2f target;
};

class PredatorPrey : public Behavior{
public:
    PredatorPrey();
    PredatorPrey(std::vector<Bot *> bots);
    PredatorPrey(std::vector<Bot*> predators, std::vector<Bot*> prey);
    virtual void update();
    int mode;
    cv::Point2f target;
    std::vector<Bot*> predators;
    std::vector<Bot*> prey;
private:
    void avoid_nearest();
    void avoid_centroid();
};


#endif //CONTROL_BEHAVIORS_H
