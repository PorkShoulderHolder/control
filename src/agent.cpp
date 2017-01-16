//
// Created by Sam Royston on 1/8/17.
//

#include "agent.h"

void StateActionSpace::init(int states, int actions) {
    this->state_count = states;
    this->action_count = actions;
    for (int i = 0; i < states; ++i) {
        std::vector<float> as;
        for (int j = 0; j < actions; ++j) {
            as.push_back(0.0f);
        }
        this->push_back(as);
    }
}

StateActionSpace::StateActionSpace(int states_x, int states_y, int actions) {
    this->count_x = states_x;
    this->count_y = states_y;
    this->init(states_x * states_y, actions);
}

StateActionSpace::StateActionSpace(std::string fn) {
    std::ifstream input_stream(fn);
    std::string line;
    int rows = 0;
    while(std::getline(input_stream, line)){
        std::stringstream ss(line);
        std::string vec_string;
        rows++;
        while(std::getline(ss, vec_string, ';')){
            std::stringstream vs(vec_string);
            float value;
            int visits;
            std::vector<StateAction> action_vals;
            while(vs >> value >> visits){
                StateAction s;
                s.value = value;
                s.visits = visits;
                action_vals.push_back(s);
            }
            this->push_back(action_vals);
        }
    }
    this->state_count = (int)this->size();
    this->action_count = (int)this->at(0).size();
    this->count_x = rows;
    this->count_y = this->state_count / rows;
}

void StateActionSpace::save(std::string fn){
    std::ofstream output_stream(fn);
    for (int i = 0; i < this->count_x; ++i) {
        for (int j = 0; j < this->count_y; ++j) {
            std::vector<StateAction> actions = this->get_action_values(i, j);
            for (StateAction action : actions) {
                output_stream << action.value << " " << action.visits;
            }
            output_stream << ";";
        }
        output_stream << "\n";
    }
    output_stream.close();
}

std::vector<StateAction> StateActionSpace::get_action_values(int state_x, int state_y){
    return this->at((unsigned long)this->count_x * state_y + state_x);
}

std::vector<StateAction> StateActionSpace::get_action_values(int state) {
    return this->at((unsigned long)state);
}

float StateActionSpace::get_action_value(int state_x, int state_y, int action) {
    std::vector<StateAction> actions = this->get_action_values(state_x, state_y);
    return actions[action].value;
}

void StateActionSpace::set_action_value(int state, int action, float value) {
    this->at((unsigned long) state).at((unsigned long) action).value = value;
}

void StateActionSpace::set_action_value(int state_x, int state_y, int action, float value) {
    this->set_action_value(this->count_x * state_y + state_x, action, value);
}

/*
 *
 *  ========================================================================================
 *
 */

Agent::Agent(const StateActionSpace &O) : Q(O) {
    int resolution = 100;
    int actions = 4;
    StateActionSpace *sa = new StateActionSpace(resolution, resolution, actions);
    this->Q = *sa;
}

Agent::Agent(const StateActionSpace &O, std::string fn) : Q(O) {
    StateActionSpace *sa = new StateActionSpace(fn);
    this->Q = *sa;

}
