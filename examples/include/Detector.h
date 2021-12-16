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
    void init_detector(ncnn::Net *net);
    void detect(const cv::Mat &bgr, std::vector <Object> &objects, ncnn::Net *net);
    cv::Mat draw_objects(const cv::Mat &bgr, const std::vector <Object> &objects);
}
