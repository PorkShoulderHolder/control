//
// Created by Sam Royston on 12/18/16.
//

#include "bot.h"
#include <sys/socket.h>
#include "network_manager.h"
#include <iostream>
#include "state.h"

const char *M_COMMAND_STR[4] = { "r:::0", "l:::0", "r:::1", "l:::1"};

COMMAND command_code(std::vector<MOTOR> command){
    if(command[0] == M_LEFT_OFF && command[1] == M_RIGHT_OFF)
        return C_BOTH_OFF;
    if(command[0] == M_LEFT_OFF && command[1] == M_RIGHT_ON)
        return C_LEFT_OFF_RIGHT_ON;
    if(command[0] == M_LEFT_ON && command[1] == M_RIGHT_OFF)
        return C_LEFT_ON_RIGHT_OFF;
    if(command[0] == M_LEFT_OFF && command[1] == M_RIGHT_OFF)
        return C_BOTH_ON;
}

std::vector<MOTOR> motor_instructions(COMMAND command_type){
    std::vector<MOTOR> c;
    switch (command_type){
        case C_BOTH_OFF:
            c.push_back(M_LEFT_OFF);
            c.push_back(M_RIGHT_OFF);
            break;
        case C_BOTH_ON:
            c.push_back(M_LEFT_ON);
            c.push_back(M_RIGHT_ON);
            break;
        case C_LEFT_OFF_RIGHT_ON:
            c.push_back(M_LEFT_OFF);
            c.push_back(M_RIGHT_ON);
            break;
        case C_LEFT_ON_RIGHT_OFF:
            c.push_back(M_LEFT_ON);
            c.push_back(M_RIGHT_OFF);
            break;
        default:
            break;
    }
    return c;
}

Bot::Bot(const char *host){
    this->port = 8888;
    this->aruco_id = NULL;
    this->host = (char *)host;
    int x_width = 500;
    int y_width = 500;
    this->action_memory = 5;
    int action_count = 4;
    const StateActionSpace *sa = new StateActionSpace(x_width, y_width, action_count);
    this->agent = new Agent(*sa);
    this->training = false;
    this->max_distance = x_width / 2.0f;
    this->target = cv::Point2d(250, 250);
    for (int i = 0; i < action_count; ++i) {
        this->info_images.push_back(cv::Mat::zeros(sa->count_x, sa->count_y, CV_32FC3));
    }
    this->load_info_image();
}

Bot::~Bot() {
    std::string id = std::to_string(this->aruco_id);
    std::string _host = std::string(this->host);
    std::string exp = std::to_string(this->agent->experience_points);
    std::string name = "device_" + id + "_" + _host + "exp" + exp;
    this->agent->serialize(name);
}

void Bot::set_target_location(cv::Point target) {
    this->target = target;
}

void Bot::load_info_image(){
    for(std::vector<StateAction> sas : this->agent->Q){
        this->set_color(sas);
    }
}

void Bot::set_color(std::vector<StateAction> sas) {
    float max_q = this->agent->Q.max_q;
    for(StateAction sa : sas){
        float r = sas[1].value / max_q;
        float g = sas[2].value / max_q;
        float b = sas[3].value / max_q;
        cv::Vec3f rgb(r, g, b);
        this->info_images[sa.action].at<cv::Vec3f>(sa.x, sa.y) = rgb;
    }
}

void Bot::set_color(std::vector<StateAction> sas, cv::Vec3f color) {
    float max_q = this->agent->Q.max_q;
    for(StateAction sa : sas){

        this->info_images[sa.action].at<cv::Vec3f>(sa.x, sa.y) = color;
    }
}

void Bot::update_image_for_state(StateAction ps, StateAction s){
    std::vector<StateAction> sas = this->agent->Q.get_action_values(s);

    this->set_color(sas, cv::Vec3f(0,0,10));

}



StateAction Bot::Q_indices(cv::Point2f location, cv::Point2f rotation){
    StateAction current_state;
    std::cout << " ____ " << location << " " << this->target << std::endl;
    double dx = location.x - this->target.x;
    double dy = location.y - this->target.y;
    double norm = cv::norm(cv::Point2d(dx, dy));
    double norm_r = cv::norm(rotation);
    dx /= norm;
    dy /= norm;
    double rot =   atan2(rotation.x / norm_r, rotation.y / norm_r) - atan2(dx, dy);
    double px = dx * cos(rot) - dy * sin(rot);
    double py = dx * sin(rot) + dy * cos(rot);
    px *= 2 * norm;
    py *= 2 * norm;
    if(isnan(px)) px = 0.0f;
    if(isnan(py)) py = 0.0f;
    current_state.x = (float)px;
    current_state.y = (float)py;
    current_state.action = 0; // to be set later
    this->current_state = current_state;
    return current_state;
};

void grid_based_learning(Bot *bot){
    cv::Point2f px((float)bot->state.location[0], (float)bot->state.location[1]);
    cv::Point2d rx = bot->state.rotation;
    StateAction current_state = bot->Q_indices(px, rx);


    if(bot->agent->experience_points == 0){

        bot->agent->update(current_state, current_state);
    }
    else{

        bot->agent->update(bot->agent->last_action, current_state);
        bot->load_info_image();
        bot->update_image_for_state(current_state, current_state);
    }

    StateAction new_s = bot->agent->act(current_state);
    std::vector<MOTOR> c = motor_instructions((COMMAND) new_s.action);
}

void Bot::learn(){
    cv::Point2f px((float)this->state.location[0], (float)this->state.location[1]);
    cv::Point2d rx = this->state.rotation;
    this->Q_indices(px, rx);
//
    if(this->agent->experience_points == 0){
//
//        this->agent->update(current_state, current_state);
    }
    else{
//
        this->load_info_image();
        this->update_image_for_state(current_state, current_state);
    }
    if(current_state.x == 0 && current_state.y == 0)
        return;
    StateAction new_s = this->train_action(current_state);

    double cost = cv::norm(cv::Point2d(current_state.y, current_state.x));
    std::cout << new_s.action << " (" << current_state.x  << ", ";
    std::cout << current_state.y << ") -> ";
    std::cout << cost << std::endl;

    std::vector<MOTOR> c = motor_instructions((COMMAND) new_s.action);
    this->apply_motor_commands(c);
}

void Bot::incr_command_queue() {
    if(this->command_queue.size() > 0){
        this->apply_motor_commands(this->command_queue.front());
        this->command_queue.pop_front();
    }
    else{
        this->learn();
    }
}

void Bot::save_to_file() {

}

cv::Point2f avg_for_range(std::deque<t_frame> deq, int start, int end, int id){
    cv::Point2f s;
    s.x = 0;
    s.y = 0;
    int obs = 0;
    for(int i = start; i < end; i++){
        t_frame frame = deq[i];
        auto current_loc = frame.locations.find(id);
        if(current_loc != frame.locations.end()){
            cv::Vec3f p = current_loc->second;
            s.x += p[0];
            s.y += p[1];
            obs ++;
        }
    }
    return s / obs;
}

void Bot::match_movement(int bot_id, int duration) {

    Bot *b = State::devices[bot_id];
    int max_id = -1;

    double max_d = 0;
    for(auto id : State::marker_ids) {
        if(State::hist.size() >= 8){

            cv::Point2f p1 = avg_for_range(State::hist, duration/2, duration, id);
            cv::Point2f p2 = avg_for_range(State::hist, 0, duration/2, id);
            double d = cv::norm(p1 - p2);
            std::cout << d << " ";
            if(d > 0 && d > max_d){
                max_id = id;
                max_d = d;
            }
        }
    }
    std::cout << b->host <<  " --> marker " << max_id << std::endl;
    b->aruco_id = max_id;
}

void Bot::apply_motor_commands(std::vector<MOTOR> commands) {
    NetworkManager *manager = new NetworkManager();
    for (int i = 0; i < 2; i++){
        manager->send_udp(this->host, (char *) M_COMMAND_STR[commands[i]], this->port);
    }
    COMMAND command_ = command_code(commands);
    this->current_state.action = command_;
    this->past_state_actions.push_front(this->current_state);
    if(this->past_state_actions.size() > this->action_memory){
        this->past_state_actions.pop_back();
    }
}

void Bot::record_motor_commands(std::vector<MOTOR> commands){
    this->apply_motor_commands(commands);

}


StateAction Bot::train_action(StateAction state_action) {
    NetworkManager *manager = new NetworkManager();
    std::string msg;
    msg += "{\"training\":" + std::to_string((int) this->training);
    msg += ",\"x\":" + std::to_string(state_action.x);
    msg += ",\"y\":" + std::to_string(state_action.y);
    int j = 0;
    for(StateAction s : this->past_state_actions){
        msg += ",\"a" + std::to_string(j) + "\":" + std::to_string(s.action);
        msg += ",\"x" + std::to_string(j) + "\":" + std::to_string(s.x);
        msg += ",\"y" + std::to_string(j) + "\":" + std::to_string(s.y);
        j++;
    }
    msg += ",\"id\":" + std::to_string(this->aruco_id) + "}";
    char buffer[1024];
    std::string brain_host = "127.0.0.1";
    char* response = manager->send_tcp((char*)brain_host.c_str(), (char*)msg.c_str(), 9999, buffer);
    state_action.action = atoi(response);
    return state_action;
}

