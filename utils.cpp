//
// Created by Sam Royston on 12/19/16.
//



#include "utils.h"
#include "state.h"


cv::Mat Utils::homogenous_quad( std::vector<cv::Point2f> quad ){
    cv::Mat out = cv::Mat::ones(3, 4, CV_32F);
    out.at<float>(0,0) = quad[0].x;
    out.at<float>(0,1) = quad[1].x;
    out.at<float>(0,2) = quad[2].x;
    out.at<float>(0,3) = quad[3].x;
    out.at<float>(1,0) = quad[0].y;
    out.at<float>(1,1) = quad[1].y;
    out.at<float>(1,2) = quad[2].y;
    out.at<float>(1,3) = quad[3].y;
    return out;
}

void Utils::compute_ground_plane(std::vector< std::vector<cv::Point2f> > quads){
    if(Utils::distortion_coefs.empty() || Utils::camera_matrix.empty()){
        cv::FileStorage fs("camera_data.xml", cv::FileStorage::READ);
        fs["Distortion_Coefficients"] >> Utils::distortion_coefs;
        fs["Camera_Matrix"] >> Utils::camera_matrix;
    }
    std::vector<cv::Vec3d> rotations;
    std::vector<cv::Vec3d> locations;
    cv::aruco::estimatePoseSingleMarkers(quads, 1.0f, Utils::camera_matrix, Utils::distortion_coefs,
                                         rotations, locations);

    for( cv::Vec3d p3  : locations ){
        cv::Point2f p(10 * p3[0] + 100, 10 * p3[1] + 100);
        cv::circle(State::display_image, p, 4, cv::Scalar(244,244,0));
    }

};


cv::Mat Utils::distortion_coefs;
cv::Mat Utils::camera_matrix;
