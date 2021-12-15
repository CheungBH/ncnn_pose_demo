//
// Created by sean on 2021/4/22.
//

#ifndef NCNN_CNNNET_H
#define NCNN_CNNNET_H

#include "net.h"

#include <vector>
#include <opencv2/core/core.hpp>

namespace cnnNet{
    std::vector<float> cnn(const cv::Mat &src, const ncnn::Net& cnnNet);
    int init_CNN(ncnn::Net* CNNNet);
    int print_topk(const std::vector<float> &cls_scores, int topk);
}

#endif //NCNN_CNNNET_H
