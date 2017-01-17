//
// Created by Sam Royston on 1/8/17.
//

#include "agent.h"

/*
 *
 *  ================================ StateActionSpace =================================
 *
 */


void StateActionSpace::init(int states, int actions) {
    this->state_count = states;
    this->action_count = actions;
    for (int i = 0; i < states; ++i) {
        std::vector<StateAction> as;
        for (int j = 0; j < actions; ++j) {
            StateAction s;
            s.visits = 0;
            s.value = 0.0f;
            s.x = i % this->count_x;
            s.y = i / this->count_x;
            s.action = j;
            as.push_back(s);
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
        int cols = 0;
        while(std::getline(ss, vec_string, ';')){
            std::stringstream vs(vec_string);
            float value;
            int visits;
            int a = 0;
            std::vector<StateAction> action_vals;
            while(vs >> value >> visits){
                StateAction s;
                s.value = value;
                s.visits = visits;
                s.x = cols;
                s.y = rows;
                s.action = a;
                action_vals.push_back(s);
                a++;
            }
            this->push_back(action_vals);
            cols ++;
        }
        rows++;
    }
    this->state_count = (int)this->size();
    this->action_count = (int)this->at(0).size();
    this->count_x = rows;
    this->count_y = this->state_count / rows;
}

void StateActionSpace::save(std::string fn){
    std::ofstream output_stream(fn);
    output_stream << std::setprecision(10);
    for (int i = 0; i < this->count_x; ++i) {
        for (int j = 0; j < this->count_y; ++j) {
            std::vector<StateAction> actions = this->get_action_values(i, j);
            int k = 0;
            for (StateAction action : actions) {
                if(k + 1 < actions.size()){
                    output_stream << action.value << " " << action.visits << " ";
                }
                else{
                    output_stream << action.value << " " << action.visits;
                }
                k++;
            }
            output_stream << ";";
        }
        output_stream << std::endl;
    }
    output_stream.close();
}

std::vector<StateAction> StateActionSpace::get_action_values(int state_x, int state_y){
    return this->at((unsigned long)this->count_y * state_x + state_y);
}

std::vector<StateAction> StateActionSpace::get_action_values(int state) {
    return this->at((unsigned long)state);
}

std::vector<StateAction> StateActionSpace::get_action_values(StateAction s) {
    return this->get_action_values(s.x, s.y);
}

bool state_action_comparator(const StateAction& l, const StateAction& r){
    return l.value < r.value;
};

StateAction StateActionSpace::get_greedy_action(StateAction s) {
    std::vector<StateAction> sa = this->get_action_values(s.x, s.y);
    StateAction max_el = *std::max_element(sa.begin(), sa.end(), state_action_comparator);
    return max_el;
}


float StateActionSpace::get_action_value(StateAction s) {
    return this->get_action_value(s.x, s.y, s.action);
}

StateAction StateActionSpace::get_state_action(StateAction s){
    /*
     * Fills in the Q-value(s) of a StateAction with only action and state components
     */
    std::vector<StateAction> actions = this->get_action_values(s.x, s.y);
    return actions[s.action];
}

float StateActionSpace::get_action_value(int state_x, int state_y, int action) {
    std::vector<StateAction> actions = this->get_action_values(state_x, state_y);
    return actions[action].value;
}

void StateActionSpace::set_action_value(int state, int action, float value) {
    StateAction s = this->at((unsigned long) state).at((unsigned long) action);
    s.value = value;
    s.visits++;
    this->at((unsigned long) state).at((unsigned long) action) = s;
}

void StateActionSpace::set_action_value(int state_x, int state_y, int action, float value) {
    this->set_action_value(this->count_y * state_x + state_y, action, value);
}

void StateActionSpace::set_action_value(StateAction s, float value) {
    this->set_action_value(s.x, s.y, s.action, value);
}

/*
 *
 *  ================================ Agent =================================
 *
 */

float default_reward(StateAction s){
    /*
     * default reward is euclidean distance
     */
    return sqrtf(powf(s.y, 2) + powf(s.x, 2));
}

Agent::Agent(const StateActionSpace &O, std::function<float(StateAction)> reward) : Q(O) {
    int resolution = 100;
    int actions = 4;
    StateActionSpace *sa = new StateActionSpace(resolution, resolution, actions);
    this->Q = *sa;
    this->reward = reward;
}

Agent::Agent(const StateActionSpace &O) : Q(O) {
    int resolution = 100;
    int actions = 4;
    StateActionSpace *sa = new StateActionSpace(resolution, resolution, actions);
    this->Q = *sa;
    this->reward = default_reward;
}

Agent::Agent(const StateActionSpace &O, std::string fn) : Q(O) {
    StateActionSpace *sa = new StateActionSpace(fn);
    this->Q = *sa;
    this->reward = default_reward;
}



void Agent::update(StateAction s0, StateAction s1) {
    /*
     * This function performs the crucial update step of the agent's Q-matrix;
     *
     * s0: state prior to taking action 'a' (only contains valid x, y, action fields)
     * s1: state after taking action specified in s0 (only contains valid x, y fields)
     *
     */
    s0 = this->Q.get_state_action(s0);
    StateAction greedy_option = this->Q.get_greedy_action(s1);
    float gradient = this->reward(s1) + this->discount * greedy_option.value - s0.value;
    float q0 = s0.value + this->learning_rate * (gradient);
    this->Q.set_action_value(s0, q0);
}
