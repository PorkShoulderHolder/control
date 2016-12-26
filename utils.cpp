//
// Created by Sam Royston on 12/19/16.
//



#include "utils.h"


cv::Mat Utils::homogenous_quad( std::vector<cv::Point2f> quad ){
    cv::Mat out = cv::Mat::ones(3, 4, CV_32F);
    out.at(0,0) = quad[0].x;
    out.at(0,1) = quad[1].x;
    out.at(0,2) = quad[2].x;
    out.at(0,3) = quad[3].x;
    out.at(1,0) = quad[0].y;
    out.at(1,1) = quad[0].y;
    out.at(1,2) = quad[0].y;
    out.at(1,3) = quad[0].y;
    return out;
}

std::vector<cv::Point2f> Utils::compute_ground_plane(std::vector< std::vector<cv::Point2f> > quads){
    for( std::vector<cv::Point2f> quad : quads ){
        cv::Mat H = Utils::homogenous_quad(quad);
        cv::Mat lh = H.colRange(0, 3);
        cv::Mat rh = H.colRange(3, 4);
        cv::Mat out;
        int completion = cv::solve(lh, rh, out);
        if(completion) {
            std::cout << out << std::endl;
        }
    }
};

