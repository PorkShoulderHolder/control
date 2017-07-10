//
// Created by Sam Royston on 7/8/17.
//

#include "behaviors.h"
#define PP_CENTROID 2
#define PP_NEAREST 3


Behavior::Behavior(std::vector<Bot*> bots) {
    this->bots = bots;

}

Behavior::Behavior() {
}

void Behavior::update() {
    /*
     * implement in subclass
     */

}



FollowTheLeader::FollowTheLeader(){
    this->target = cv::Point2f(0,0);
}


FollowTheLeader::FollowTheLeader(cv::Point2f t){
    this->target = t;
}

void FollowTheLeader::update() {
    this->bots.back()->target = this->target;
    for (int i = 0; i < this->bots.size() - 1; ++i) {
        Bot *b = this->bots[i];
        Bot *next_b = this->bots[i + 1];
        b->target = cv::Point2f((float)next_b->state.location[0], (float)next_b->state.location[1]);
    }
}


PredatorPrey::PredatorPrey() {

}

PredatorPrey::PredatorPrey(std::vector<Bot *> predators, std::vector<Bot *> prey) {
    this->predators = predators;
    this->prey = prey;
    this->mode = PP_NEAREST;
    for(Bot *b : predators){
        b->evasive = false;
    }

    for(Bot *b : prey){
        b->evasive = true;
    }
}


PredatorPrey::PredatorPrey(std::vector<Bot *> bots) {
    this->predators.push_back(bots.front());
    for (int i = 1; i < bots.size(); ++i) {
        this->prey.push_back(bots[i]);
    }
    for(Bot *b : predators){
        b->evasive = false;
    }

    for(Bot *b : prey){
        b->evasive = true;
    }
}

void PredatorPrey::avoid_nearest(){
    for(Bot *b : prey){
        b->target = Utils::find_closest(this->predators, b);
    }
    for(Bot *b : predators){
        b->target = Utils::find_closest(this->prey, b);
    }
}
void PredatorPrey::avoid_centroid(){
    cv::Point2f predator_centroid = Utils::centroid(this->predators);
    cv::Point2f prey_centroid = Utils::centroid(this->prey);

    for(Bot *b : prey){
        b->target = predator_centroid;
    }
    for(Bot *b : predators) {
        b->target = prey_centroid;
    }
}
void PredatorPrey::update() {
    switch(this->mode){
        case PP_NEAREST:
            this->avoid_nearest();
        case PP_CENTROID:
            this->avoid_centroid();
        default:
            this->avoid_nearest();
    }
}


