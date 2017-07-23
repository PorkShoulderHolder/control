//
// Created by Sam Royston on 1/8/17.
//

#include <vector>
#include <deque>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <unordered_map>

#ifndef CONTROL_AGENT_H
#define CONTROL_AGENT_H

struct StateAction{
    float y;
    float x;
    int action;
    float value;
    int visits;
};

class StateActionSpace : public std::vector<std::vector<StateAction> >{
public:
    typedef std::vector<std::vector<StateAction> > state_vector;

    StateActionSpace(int states_x, int states_y, int actions);
    StateActionSpace(std::string fn);
    std::vector<StateAction> get_action_values(int state);
    std::vector<StateAction> get_action_values(int state_x, int state_y);
    std::vector<StateAction> get_action_values(StateAction s);
    StateAction get_greedy_action(StateAction s);
    StateAction get_state_action(StateAction s);
    float get_action_value(int state_x, int state_y, int action);
    float get_action_value(StateAction s);

    void set_action_value(int state_x, int state_y, int action, float value);
    void set_action_value(int state, int action, float value);
    void set_action_value(StateAction s, float value);
    void save(std::string fn);

    using state_vector::at;
    using state_vector::size;

    int state_count;
    int count_x;
    float max_q;
    int count_y;
    int action_count;
private:
    void init(int states, int actions);
};



class Agent {
public:
    Agent(const StateActionSpace &O, std::function<float(StateAction)> reward);
    Agent(const StateActionSpace &O, std::string fn);
    Agent(const StateActionSpace &O);

    void serialize(std::string fn);
    void update(StateAction s0, StateAction s1, StateAction raw);

    StateAction act(StateAction s);
    float learning_rate;
    float discount;
    float eps;
    int experience_points;
    std::function<float(StateAction)> reward;
    std::function<StateAction(StateAction)> transform;
    StateActionSpace Q;
private:
    std::deque<StateAction> state_queue;

};


#endif //CONTROL_AGENT_H
