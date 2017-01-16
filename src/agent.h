//
// Created by Sam Royston on 1/8/17.
//

#include <vector>
#include <deque>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <unordered_map>

#ifndef CONTROL_AGENT_H
#define CONTROL_AGENT_H

struct StateAction{
    float value;
    int visits;
};

class StateActionSpace : private std::vector<std::vector<StateAction>>{
public:
    StateActionSpace(int states_x, int states_y, int actions);
    StateActionSpace(std::string fn);
    std::vector<StateAction> get_action_values(int state);
    std::vector<StateAction> get_action_values(int state_x, int state_y);
    float get_action_value(int state_x, int state_y, int action);
    void set_action_value(int state_x, int state_y, int action, float value);
    void set_action_value(int state, int action, float value);
    void save(std::string fn);

    int state_count;
    int count_x;
    int count_y;
    int action_count;

private:
    void init(int states, int actions);
};

class Agent {
public:
    Agent::Agent(const StateActionSpace &O);
    Agent(const StateActionSpace &O, std::string fn);
    void serialize(std::string fn);

private:
    StateActionSpace Q;

};


#endif //CONTROL_AGENT_H
