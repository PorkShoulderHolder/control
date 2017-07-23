//
// Created by Sam Royston on 7/22/17.
//

#ifndef CONTROL_BEHAVIOR_BASE_H
#define CONTROL_BEHAVIOR_BASE_H

#include "bot.h"

class Behavior {
public:
    Behavior();
    Behavior(std::vector<Bot*> bots);
    virtual void update();
    cv::Point2f target;
    std::vector<Bot*> bots;
};



#endif //CONTROL_BEHAVIOR_BASE_H
