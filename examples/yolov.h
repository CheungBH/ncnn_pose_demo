//
// Created by sean on 2021/4/8.
//
#pragma once

#include "net.h"
//#include "RegionProcessor.h"

#include <opencv2/core/mat.hpp>

struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

namespace yolov {
    int init_yolov4(ncnn::Net *yolov4, int *target_size);

    int detect_yolov4(cv::Mat bgr, std::vector <Object> &objects, int target_size, ncnn::Net *yolov4);

    void draw_objects(cv::Mat& bgr, const std::vector <Object> &objects, int is_streaming);

    int detect_padded_yolov4(const cv::Mat& bgr, std::vector<Object>& objects, int target_size, double resize_ratio, double orig_w, double orig_h, ncnn::Net* yolov4);
}
