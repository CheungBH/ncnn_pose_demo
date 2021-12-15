//
// Created by sean on 2021/4/8.
//
#pragma once

#include "net.h"
#include <opencv2/core/mat.hpp>

struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

namespace yolov {
    int init_yolov4(ncnn::Net *yolov4);

    int detect_yolov4(const cv::Mat &bgr, std::vector <Object> &objects, int target_size, ncnn::Net *yolov4);

    cv::Mat draw_objects(const cv::Mat &bgr, const std::vector <Object> &objects);

}
