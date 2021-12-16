//
// Created by hkuit164 on 16/12/2021.
//

#ifndef NCNN_NANODET_H
#define NCNN_NANODET_H

#endif //NCNN_NANODET_H
#include "yolov.h"

namespace nanodet{
    int detect_nanodet(ncnn::Net* nanodet, const cv::Mat& bgr, std::vector<Object>& objects, int target_size);
    int init_nanodet(ncnn::Net* detector);
    inline float intersection_area(const Object& a, const Object& b);
    void qsort_descent_inplace(std::vector<Object>& faceobjects, int left, int right);
    void qsort_descent_inplace(std::vector<Object>& faceobjects);
    void nms_sorted_bboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold);
    void generate_proposals(const ncnn::Mat& cls_pred, const ncnn::Mat& dis_pred, int stride, const ncnn::Mat& in_pad, float prob_threshold, std::vector<Object>& objects);
}
