//
// Created by Sam Royston on 12/19/16.
//



#include <fstream>
#include "utils.h"
#include "state.h"
#define DEVICE_FILE "domains.txt"
#define CAMERA_PARAMS_FILE "configuration_files/setting.xml"




enum PATTERN_TYPE{
    PATTERN_TYPE_ITER,
    PATTERN_TYPE_TREE
};

enum INIT_SPEED{
    INIT_SPEED_SLOW = 16,
    INIT_SPEED_NORMAL = 8,
    INIT_SPEED_FAST = 4
};

cv::Point2d Utils::getRotationFromQuad(std::vector<cv::Point2f> quad ){
    cv::Point2f center = quad[0] + quad[1] + quad[2] + quad[3];
    center /=4;
    cv::Point2f r = center - quad[0];
    r = r / cv::norm(r);
    float radians = 1.3; // rotate direction vector
    double sn = sin(radians * M_PI);
    double cs = cos(radians * M_PI);
    cv::Point2d rot(r.x * cs - r.y * sn, r.x * sn + r.y * cs);
    return rot;
}

LocationRotationVec Utils::compute_ground_plane(std::vector< std::vector<cv::Point2f> > quads){
    State *S = State::shared_instance();
    if(Utils::distortion_coefs.empty() || Utils::camera_matrix.empty()){
        cv::FileStorage fs(CAMERA_PARAMS_FILE, cv::FileStorage::READ);
        fs["Distortion_Coefficients"] >> Utils::distortion_coefs;
        fs["Camera_Matrix"] >> Utils::camera_matrix;
    }
    std::vector<cv::Vec3d> rotations;
    std::vector<cv::Vec3d> locations;
    cv::aruco::estimatePoseSingleMarkers(quads, 1.0f, Utils::camera_matrix, Utils::distortion_coefs,
                                         rotations, locations);
    std::vector<cv::Point2d> rotns;
    int i = 0;
    for( cv::Vec3d p3  : locations ){
        cv::Point2d p(8.0f * p3[0] + 800, 8.0f * p3[1] + 800);
        cv::Point2d rot = Utils::getRotationFromQuad(quads[i]);
        rotns.push_back(rot);
        cv::Point2d r(p.x + 20 * rot.x, p.y + 20 * rot.y);
        cv::circle(S->display_image, p, 10, cv::Scalar(244,244,0), 5);
        cv::line(S->display_image, p, r, cv::Scalar(0,244,244), 5);
        i++;
    }

    for( Bot *b  : S->devices ){
        cv::Point2d g(8.0f * b->target.x + 800, 8.0f * b->target.y + 800);

        cv::circle(S->display_image, g, 20, cv::Scalar(244, 0, 0), 8);
    }

    LocationRotationVec p(locations, rotns);
    return p;
};


std::vector<char *> Utils::get_device_names_from_file(){
    std::ifstream infile("domains.txt");
    std::string line;
    std::vector<char *> out;
    while(infile >> line){
        out.push_back( strdup(line.c_str()) );
    }
    return out;
}

std::deque< std::vector<MOTOR> > command_seq_for_index(int index, int count, INIT_SPEED speed, PATTERN_TYPE pattern){
    cv::Mat command_mat = cv::Mat::zeros(2, (count + 6) * speed, CV_32F);
    std::deque< std::vector<MOTOR> > command_deq;
    switch(pattern){
        case PATTERN_TYPE_ITER:
            command_mat(cv::Rect((2 + index) * speed, 0, speed, 1)) = 1;
        default:
            command_mat(cv::Rect((2 + index) * speed, 0, speed, 1)) = 1;
    }
    for(int n = 0; n < command_mat.cols; n++)
    {
        cv::Mat c = command_mat.col(n);

        std::vector<MOTOR> command;
        command.push_back(c.at<float>(0) != 0 ? M_LEFT_ON : M_LEFT_OFF);
        command.push_back(c.at<float>(1) != 0 ? M_RIGHT_ON : M_RIGHT_OFF);
        command_deq.push_back(command);
    }

    return command_deq;
}

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
    switch (command_type) {
        case C_BOTH_OFF: {
            c.push_back(M_LEFT_OFF);
            c.push_back(M_RIGHT_OFF);
            break;
        }
        case C_BOTH_ON: {
            c.push_back(M_LEFT_ON);
            c.push_back(M_RIGHT_ON);
            break;
        }
        case C_LEFT_OFF_RIGHT_ON:{
            c.push_back(M_LEFT_OFF);
            c.push_back(M_RIGHT_ON);
            break;
        }
        case C_LEFT_ON_RIGHT_OFF: {
            c.push_back(M_LEFT_ON);
            c.push_back(M_RIGHT_OFF);
            break;
        }
        default:
            break;
    }
    return c;
}


std::deque<MOTOR*> command_seq_for_index(int index, int count){
    command_seq_for_index(index, count, INIT_SPEED_NORMAL, PATTERN_TYPE_ITER);
}

void test_task_15(void){
    std::cout << "15 frames" << std::endl;
}

void test_task_30(void){
    std::cout << "30 frames" << std::endl;
}

void end_match_aruco(){

}

cv::Point2f Utils::find_closest(std::vector<Bot *> bots, Bot *bot){
    double min_dist = 10000000;
    cv::Point2f closest;
    cv::Point2f p0 = cv::Point2f((float)bot->state.location[0], (float)bot->state.location[1]);
    for(Bot *b : bots){
        cv::Point2f p1 = cv::Point2f((float)b->state.location[0], (float)b->state.location[1]);
        std::vector<cv::Point2f> pts;
        pts.push_back(p1 - p0);
        double d = cv::norm(pts, cv::NORM_L2SQR);
        if(d < min_dist){
            min_dist = d;
            closest = p1;
        }

    }
    return closest;
}

cv::Point2f Utils::centroid(std::vector<Bot *> bots){
    cv::Point2f centroid(0.0f, 0.0f);
    for(Bot *b : bots){
        centroid.x += (float)b->state.location[0];
        centroid.y += (float)b->state.location[1];
    }
    centroid = centroid * (1.0f / bots.size());
    return centroid;
}

std::pair<int, int> Utils::get_lr(char *bot_name){
    std::ifstream setting_file(BOT_SETTINGS_FILE);
    std::string line;

    std::string bot_str(bot_name);
    while (std::getline(setting_file, line)) {
        std::stringstream line_stream(line);
        std::string hostname;
        int left;
        int right;

        while(line_stream >> hostname >> left >> right){
            if(hostname.compare(bot_str) == 0){
                std::cout << line << std::endl;
                setting_file.close();
                return std::pair<int, int>(left, right);
            }

        }
    }
    std::cout << "host name " + bot_str + " not found in config file \n";
    return std::pair<int, int>(-1, -1);
}

bool Utils::begin_match_aruco() {
    //TODO: finish
    State *S = State::shared_instance();
    int i = 0;
    INIT_SPEED speed = INIT_SPEED_SLOW;

    for( Bot *bot : S->devices ){
        bot->command_queue = command_seq_for_index(i, (int)S->devices.size(), speed, PATTERN_TYPE_ITER);
        task t;
        t.id = i;
        t.duration = speed;
        t.f = &(Bot::match_movement);
        S->schedule_task(t, (i + 1 + 2) * speed);
        i++;
    }
}




cv::Mat Utils::distortion_coefs;
cv::Mat Utils::camera_matrix;
