//
// Created by hkuit164 on 16/12/2021.
//

#ifndef NCNN_DETECTOR_H
#define NCNN_DETECTOR_H
#endif //NCNN_DETECTOR_H


#include "yolov.h"
#include "nanodet.h"

namespace Detector{
//    std::string detector_type;
//    int input_size;
//    bool loaded_det;
    std::vector<Object> select_objects(std::vector<Object>, int image_height, int image_width);
    std::vector<Object> scale(std::vector<Object> objects, int image_height, int image_width);
    std::vector<Object> select_center(std::vector<Object> objects, int image_height, int image_width);
    std::vector<Object> select_largest(std::vector<Object> objects);
    void init_detector(ncnn::Net *net);
    void detect(const cv::Mat &bgr, std::vector <Object> &objects, ncnn::Net *net);
    float calculate_distance(cv::Point p1, cv::Point p2);
    cv::Mat draw_objects(const cv::Mat &bgr, const std::vector <Object> &objects);
}
