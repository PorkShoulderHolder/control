//
// Created by Sam Royston on 12/19/16.
//



#include <fstream>
#include "utils.h"
#include "state.h"
#define DEVICE_FILE "domains.txt"

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

    if(Utils::distortion_coefs.empty() || Utils::camera_matrix.empty()){
        cv::FileStorage fs("camera_data.xml", cv::FileStorage::READ);
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
        cv::Point2d p(4.0f * p3[0] + 400, 4.0f * p3[1] + 400);
        cv::Point2d rot = Utils::getRotationFromQuad(quads[i]);
        rotns.push_back(rot);
        cv::Point2d r(p.x + 20 * rot.x, p.y + 20 * rot.y);
        cv::circle(State::display_image, p, 10, cv::Scalar(244,244,0), 5);
        cv::line(State::display_image, p, r, cv::Scalar(0,244,244), 5);
        i++;
    }

    LocationRotationVec p(locations, rotns);
    return p;
};


std::vector<char *> Utils::get_device_names_from_file(){
    std::ifstream infile(DEVICE_FILE);
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

bool Utils::begin_match_aruco() {
    //TODO: finish
    int i = 0;
    INIT_SPEED speed = INIT_SPEED_SLOW;

    for( Bot *bot : State::devices ){
        bot->command_queue = command_seq_for_index(i, (int)State::devices.size(), speed, PATTERN_TYPE_ITER);
        task t;
        t.id = i;
        t.duration = speed;
        t.f = &(Bot::match_movement);
        State::schedule_task(t, (i + 1 + 2) * speed);
        i++;
    }
}




cv::Mat Utils::distortion_coefs;
cv::Mat Utils::camera_matrix;
