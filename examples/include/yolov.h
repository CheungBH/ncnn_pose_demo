//
// Created by sean on 2021/4/8.
//
#ifndef NCNN_YOLOV_H
#define NCNN_TOLOV_H
#endif

#pragma once

#include "net.h"
#include <opencv2/core/mat.hpp>
#include "utils.h"
#include "ConsoleVariableSystem.h"
#include <algorithm>
#include <chrono>

struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

namespace yolov {
    int init_yolov4(ncnn::Net *yolov4);
    int detect_yolov4(const cv::Mat &bgr, std::vector <Object> &objects, int target_size, ncnn::Net *yolov4);
//    cv::Mat draw_objects(const cv::Mat &bgr, const std::vector <Object> &objects);
}
