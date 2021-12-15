//
// Created by sean on 2021/4/22.
//

#ifndef NCNN_SPPENET_H
#define NCNN_SPPENET_H

#include "net.h"
#include "yolov.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <opencv2/core/mat.hpp>

struct KP
{
    cv::Point2f p;
    float prob;
};

namespace sppeNet {
    void cropImageOriginal(std::vector<cv::Mat> &target, const cv::Mat &src, const std::vector<Object> &obj);
    std::vector <KP> sppeOneAll(const cv::Mat &src, const ncnn::Net &sppeNet, const cv::Rect& box);
    void draw_pose(const cv::Mat &bgr, const std::vector <KP> &keypoints);
    int init_sppe(ncnn::Net* sppeNet);
}

#endif //NCNN_SPPENET_H
