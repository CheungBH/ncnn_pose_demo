#pragma once

#ifndef NCNN_TRACKER_H
#define NCNN_TRACKER_H

#endif

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <sstream>

//Image
#include <opencv2/opencv.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#include "Hungarian.h"
#include "TrackingBox.h"
#include "KalmanTracker.h"



struct MatchItems {
    std::set<int> unmatchedDet;
    std::set<int> unmatchedTracker;
    std::vector<cv::Point> matchedPairs;
};

std::vector<cv::Rect_<float>> get_predictions();
double GetIOU(cv::Rect_<float> bb_dr, cv::Rect_<float> bb_gt);
MatchItems Sort_match(std::vector<cv::Rect> bboxes, std::vector<cv::Rect_<float>> predictedBoxes);
std::vector<TrackingBox> update_trackers(std::vector<cv::Rect> bboxes, MatchItems M_items);
void update_dataFrame(int f_num, std::vector<vector<float>> bbox);
std::vector<TrackingBox> get_first_frame_result(std::vector<cv::Rect> bboxes);
std::vector<TrackingBox> SORT(std::vector<cv::Rect> bboxes);
void vis_id(std::vector<TrackingBox> tracking_result, cv::Mat frm);
