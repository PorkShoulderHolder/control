//
// Created by Sam Royston on 12/18/16.
//

#include "bot.h"
#include <sys/socket.h>
#include "network_manager.h"
#include <iostream>
#include "state.h"

const char *M_COMMAND_STR[4] = { "r:::0", "l:::0", "r:::1", "l:::1"};

Bot::Bot(const char *host){
    this->port = 8888;
    this->aruco_id = NULL;
    this->host = (char *)host;
    int x_width = 100;
    int y_width = 100;
    int action_count = 4;
    const StateActionSpace *sa = new StateActionSpace(x_width, y_width, action_count);
    this->agent = new Agent(*sa);
    this->max_distance = 0.4f;
    this->target = cv::Point(5, 10);
    for (int i = 0; i < action_count; ++i) {
        this->info_images.push_back(cv::Mat::ones(sa->count_x, sa->count_y, CV_32F));
    }
}

Bot::~Bot() {
    std::string id = std::to_string(this->aruco_id);
    std::string _host = std::string(this->host);
    std::string exp = std::to_string(this->agent->experience_points);
    std::string name = "device_" + id + "_" + host + "exp" + exp;
    this->agent->serialize(name);
}

void Bot::set_target_location(cv::Point target) {
    this->target = target;
}

void Bot::update_image_for_state(StateAction s){
    std::vector<StateAction> sas = this->agent->Q.get_action_values(s);
    for(StateAction sa : sas){
        this->info_images[sa.action].at<float>(sa.x, sa.y) = sa.value;
    }
}



StateAction Bot::Q_indices(cv::Point2f location, cv::Point2f rotation){
    StateAction current_state;
    float dx = location.x - this->target.x;
    float dy = location.y - this->target.y;
    dx /= this->max_distance;
    dy /= this->max_distance;
    dx = min(dx, 99.0f);
    dx = max(dx, 1.0f);
    dy = min(dy, 99.0f);
    dy = max(dy, 1.0f);
    current_state.x = (int)(dx); // + this->max_distance / 2);
    current_state.y = (int)(dy);//+ this->max_distance / 2);
    current_state.action = 0;//+ this->max_distance / 2);
    std::cout << dx << " " << dy << std::endl;
    std::cout << this->target << std::endl;
    return current_state;
};

void Bot::learn(){
    cv::Point2f px((float)this->state.location[0], (float)this->state.location[1]);
    cv::Point2d rx = this->state.rotation;
    StateAction current_state = this->Q_indices(px, rx);

    this->update_image_for_state(current_state);

    if(this->agent->experience_points == 0){

        this->agent->update(current_state, current_state);
    }
    else{

        this->agent->update(this->agent->last_action, current_state);
    }

    StateAction new_s = this->agent->act(current_state);
    std::vector<MOTOR> c;
    std::vector<MOTOR> c1;
    std::vector<MOTOR> c2;
    std::vector<MOTOR> c3;

    switch (new_s.action){
        case 0:
            c.push_back(M_LEFT_OFF);
            c.push_back(M_RIGHT_OFF);
            this->apply_motor_commands(c);
            break;
        case 1:
            c1.push_back(M_LEFT_ON);
            c1.push_back(M_RIGHT_ON);
            this->apply_motor_commands(c1);
            break;
        case 2:
            c2.push_back(M_LEFT_OFF);
            c2.push_back(M_RIGHT_ON);
            this->apply_motor_commands(c2);
            break;
        case 3:
            c3.push_back(M_LEFT_ON);
            c3.push_back(M_RIGHT_OFF);
            this->apply_motor_commands(c3);
            break;
        default:
            break;
    }
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
        manager->send_to(this->host, (char *)M_COMMAND_STR[commands[i]], this->port);
    }
}


