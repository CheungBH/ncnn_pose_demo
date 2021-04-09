//
// Created by sean on 2021/4/8.
//
#pragma once

#include "net.h"
#include "RegionProcessor.h"

#include <opencv2/core/core.hpp>

struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

struct KP
{
    cv::Point2f p;
    float prob;
};

namespace ncnn_ai {
    int init_yolov4(ncnn::Net *yolov4, int *target_size);

    int detect_yolov4(const cv::Mat &bgr, std::vector <Object> &objects, int target_size, ncnn::Net *yolov4);

    cv::Mat draw_objects(const cv::Mat &bgr, const std::vector <Object> &objects, int is_streaming);

    void cropImageFrom(std::vector <cv::Mat> &target, const cv::Mat &src, const std::vector <Object> &obj);

    std::vector <KP> sppeOne(const cv::Mat &src, const ncnn::Net& sppeNet);

    void draw_pose(const cv::Mat &bgr, const std::vector <KP> &keypoints, int is_streaming);

    std::vector<float> cnn(const cv::Mat &src, const ncnn::Net& cnnNet);

    int print_topk(const std::vector<float> &cls_scores, int topk);
}