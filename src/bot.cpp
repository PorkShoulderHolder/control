//
// Created by Sam Royston on 12/18/16.
//

#include "bot.h"
#include "network_manager.h"
#include "state.h"

#define TIME_CONCAT 1498414000000
const char *M_COMMAND_STR[4] = { "r:::0", "l:::0", "r:::1", "l:::1"};


Bot::Bot(const char *host){
    this->port = 8888;
    this->aruco_id = NULL;
    this->host = (char *)host;
    int x_width = 500;
    int y_width = 500;
    this->action_memory = 50;
    int action_count = 4;
    const StateActionSpace *sa = new StateActionSpace(x_width, y_width, action_count);
    this->agent = new Agent(*sa);
    this->training = false;
    this->target = cv::Point2d(250, 250);
    this->reached_target_thresh = 3.4f;
    for (int i = 0; i < action_count; ++i) {
        this->info_images.push_back(cv::Mat::zeros(sa->count_x, sa->count_y, CV_32FC3));
    }
    this->evasive = false;
    std::pair<int, int> lr = Utils::get_lr(this->host);
    this->right_command = lr.first;
    this->left_command = lr.second;
    this->load_info_image();
}

Bot::~Bot() {
    std::string id = std::to_string(this->aruco_id);
    std::string _host = std::string(this->host);
    std::string exp = std::to_string(this->agent->experience_points);
    std::string name = "device_" + id + "_" + _host + "exp" + exp;
    this->agent->serialize(name);
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
        this->info_images[sa.action].at<cv::Vec3f>((int)sa.x, (int)sa.y) = rgb;
    }
}

void Bot::set_color(std::vector<StateAction> sas, cv::Vec3f color) {
    for(StateAction sa : sas){
        this->info_images[sa.action].at<cv::Vec3f>((int)sa.x, (int)sa.y) = color;
    }
}

void Bot::update_image_for_state(StateAction s){

    std::vector<StateAction> sas = this->agent->Q.get_action_values(s);
    this->set_color(sas, cv::Vec3f(0,0,10));
}



StateAction Bot::Q_indices(cv::Point2f location, cv::Point2f rotation){
    /*
     * parameterize target and state in terms of a single reference frame
     *
     */
    StateAction state;
    double dx = location.x - this->target.x;
    double dy = location.y - this->target.y;
    cv::Point2d d(dx, dy);
    double norm = cv::norm(d);
    double norm_r = cv::norm(rotation);
    double cos_theta = d.dot(rotation) / (norm * norm_r);
    double theta = acos(cos_theta);
    double cross = rotation.cross(d);
    if( cross == -0.0f ){
        theta = M_PI;
    }
    else if( cross < 0.0f){
        theta = (2 * M_PI) - theta;
    }

    double new_x = 0 * cos(theta) - 1 * sin(theta);
    double new_y =  0  * sin(theta) + 1 * cos(theta);
    new_x *= norm;
    new_y *= norm;
    double px = new_x;
    double py = new_y;
    if(isnan(px)) px = 0.0f;
    if(isnan(py)) py = 0.0f;
    state.x = (float)px;
    state.y = (float)py;
    cv::Point2d center(200, 200);
    cv::Point2d target(200 + px * 3.5, 200 + py * 3.5);
    cv::circle(State::display_image, center, 5, cv::Scalar(0,244,0), 5);
    cv::circle(State::display_image, target, 5, cv::Scalar(0,244,244), 5);
    cv::line(State::display_image, center, target, cv::Scalar(0,244,0), 5);

    state.action = 0; // to be set later
    return state;
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
        bot->update_image_for_state(current_state);
    }

    StateAction new_s = bot->agent->act(current_state);
    std::vector<MOTOR> c = motor_instructions((COMMAND) new_s.action);
}

void Bot::act(){
    /*
     *  Compute current state with repsect to currnt target and
     *  send a new command to the device based on the logic in control_action()
     */
    cv::Point2f px((float)this->state.location[0], (float)this->state.location[1]);
    cv::Point2d rx = this->state.rotation;
    this->current_state = this->Q_indices(px, rx);

    if(this->current_state.x == 0 && this->current_state.y == 0)
        return;
    StateAction new_s = this->control_action(this->current_state);

    std::vector<MOTOR> c = motor_instructions((COMMAND) new_s.action);
    this->apply_motor_commands(c);
}

void Bot::incr_command_queue() {
    /*
     * send commands according to the command override queue if is not empty, otherwise
     * continue down the default execution path (respond to current target)
     */
    cv::Point2f px((float)this->state.location[0], (float)this->state.location[1]);
    cv::Point2d rx = this->state.rotation;
    this->current_state = this->Q_indices(px, rx);
    if(this->command_queue.size() > 0){
        this->apply_motor_commands(this->command_queue.front());
        if(this->training){
            COMMAND c = command_code(this->command_queue.front());
            this->current_state.action = c;
            this->control_action(this->current_state);
        }
        this->command_queue.pop_front();
    }
    else{
        this->act();
    }
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
    std::pair<StateAction, long int> sat;
    sat.first = this->current_state;
    sat.second = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                                                                               .time_since_epoch()).count();
    this->past_state_actions.push_front(sat);
    if(this->past_state_actions.size() > this->action_memory){
        this->past_state_actions.pop_back();
    }
}

std::string Bot::send_state(StateAction state_action) {
    /*
     * send current state to external processing server and return the response
     */
    NetworkManager *manager = new NetworkManager();
    std::string msg;

    msg += "{\"training\":" + std::to_string((int) this->training);
    msg += ",\"x\":" + std::to_string(state_action.x);
    msg += ",\"y\":" + std::to_string(state_action.y);
    int j = 0;
    long int now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                                                                                 .time_since_epoch()).count();

    msg += ",\"t\":" + std::to_string(now - TIME_CONCAT);
    for(std::pair<StateAction, long int> sat : this->past_state_actions){
        StateAction s = sat.first;
        msg += ",\"a" + std::to_string(j) + "\":" + std::to_string(s.action);
        msg += ",\"x" + std::to_string(j) + "\":" + std::to_string(s.x);
        msg += ",\"y" + std::to_string(j) + "\":" + std::to_string(s.y);
        msg += ",\"t" + std::to_string(j) + "\":" + std::to_string(sat.second - TIME_CONCAT);
        j++;
    }
    msg += ",\"id\":" + std::to_string(this->aruco_id) + "}";
    char buffer[1024];
    std::string brain_host = "127.0.0.1";
    char* response = manager->send_tcp((char*)brain_host.c_str(), (char*)msg.c_str(), 9998, buffer);
    return std::string(response);
}

void Bot::append_path(std::vector< cv::Point2d > path){
    for(cv::Point2d target : path){
        this->target_queue.push_back(target);
    }
}

void Bot::replace_path(std::vector< cv::Point2d > path){
    this->target_queue.clear();
    this->append_path(path);
}

StateAction Bot::control_action(StateAction state_action) {
    /*
     * basic heuristic control governing movement towards or away the target
     */
    int fight_or_flight = this->evasive ? -1 : 1;
    if(fight_or_flight * state_action.x > 0){
        state_action.action = this->right_command;
    }
    else{
        state_action.action = this->left_command;
    }
    if(cv::norm(cv::Point2d(state_action.x, state_action.y)) < this->reached_target_thresh) {
        state_action.action = C_BOTH_OFF;
        if (this->target_queue.size() > 0){
            this->target = this->target_queue.front();
            this->target_queue.pop_front();
        }
    }
    return state_action;
}

