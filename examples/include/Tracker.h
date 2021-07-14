#pragma once
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


int max_age = 90;//max time object disappear
int min_hits = 3; //min time target appear
double iouThreshold = 0.3;//matching IOU
std::vector<KalmanTracker> trackers;
std::vector<std::vector<TrackingBox>> detFrameData;
std::vector<TrackingBox> SORT(std::vector<vector<float>> bbox, int fi);

struct MatchItems {
    std::set<int> unmatchedDet;
    std::set<int> unmatchedTracker;
    std::vector<cv::Point> matchedPairs;
};

std::vector<cv::Rect_<float>> get_predictions() {
    std::vector<cv::Rect_<float>> predBoxes;
    for (auto it = trackers.begin(); it != trackers.end();)
    {
        cv::Rect_<float> pBox = (*it).predict();
        //std::cout << pBox.x << " " << pBox.y << std::endl;
        if (pBox.x >= 0 && pBox.y >= 0)
        {
            predBoxes.push_back(pBox);
            it++;
        }
        else
        {
            it = trackers.erase(it);
            //cerr << "Box invalid at frame: " << frame_count << endl;
        }
    }
    return predBoxes;
}

double GetIOU(cv::Rect_<float> bb_dr, cv::Rect_<float> bb_gt) {
    float in = (bb_dr & bb_gt).area();
    float un = bb_dr.area() + bb_gt.area() - in;

    if (un < DBL_EPSILON)
        return 0;

    double iou = in / un;

    return iou;
}


MatchItems Sort_match(std::vector<cv::Rect> bboxes, std::vector<cv::Rect_<float>> predictedBoxes) {

    unsigned int trkNum = 0;
    unsigned int detNum = 0;
    trkNum = predictedBoxes.size();
    detNum = bboxes.size();

    std::vector<std::vector<double>> iouMatrix;
    std::vector<int> assignment;

    std::set<int> unmatchedDetections;
    std::set<int> unmatchedTrajectories;
    std::set<int> allItems;
    std::set<int> matchedItems;
    // result
    std::vector<cv::Point> matchedPairs;
    MatchItems matched_result;

    iouMatrix.resize(trkNum, std::vector<double>(detNum, 0));


    for (unsigned int i = 0; i < trkNum; i++) // compute iou matrix as a distance matrix
    {
        for (unsigned int j = 0; j < detNum; j++)
        {
            // use 1-iou because the hungarian algorithm computes a minimum-cost assignment.
            iouMatrix[i][j] = 1 - GetIOU(predictedBoxes[i], bboxes[j]);
        }
    }

    // solve the assignment problem using hungarian algorithm.
    // the resulting assignment is [track(prediction) : detection], with len=preNum
    HungarianAlgorithm HungAlgo;
    HungAlgo.Solve(iouMatrix, assignment);

    // find matches, unmatched_detections and unmatched_predictions
    if (detNum > trkNum) //	there are unmatched detections
    {
        for (unsigned int n = 0; n < detNum; n++)
            allItems.insert(n);

        for (unsigned int i = 0; i < trkNum; ++i)
            matchedItems.insert(assignment[i]);

        // calculate the difference between allItems and matchedItems, return to unmatchedDetections
        std::set_difference(allItems.begin(), allItems.end(),
                            matchedItems.begin(), matchedItems.end(),
                            insert_iterator<set<int>>(unmatchedDetections, unmatchedDetections.begin()));
    }
    else if (detNum < trkNum) // there are unmatched trajectory/predictions
    {
        for (unsigned int i = 0; i < trkNum; ++i)
            if (assignment[i] == -1) // unassigned label will be set as -1 in the assignment algorithm
                unmatchedTrajectories.insert(i);
    }

    // filter out matched with low IOU
    // output matchedPairs
    for (unsigned int i = 0; i < trkNum; ++i)
    {
        if (assignment[i] == -1) // pass over invalid values
            continue;
        if (1 - iouMatrix[i][assignment[i]] < iouThreshold)
        {
            unmatchedTrajectories.insert(i);
            unmatchedDetections.insert(assignment[i]);
        }
        else
            matchedPairs.push_back(cv::Point(i, assignment[i]));
    }
    matched_result.matchedPairs = matchedPairs;
    matched_result.unmatchedDet = unmatchedDetections;
    matched_result.unmatchedTracker = unmatchedTrajectories;
    return matched_result;
};


std::vector<TrackingBox> update_trackers(std::vector<cv::Rect> bboxes, MatchItems M_items) {

    std::vector<TrackingBox> Sort_result;
    std::vector<cv::Point> matchedPairs = M_items.matchedPairs;
    std::set<int> unmatchedDetections = M_items.unmatchedDet;

    int detIdx, trkIdx;
    for (unsigned int i = 0; i < matchedPairs.size(); i++)
    {
        trkIdx = matchedPairs[i].x;
        detIdx = matchedPairs[i].y;
        trackers[trkIdx].update(bboxes[detIdx]);
    }

    // create and initialize new trackers for unmatched detections
    for (auto umd : unmatchedDetections)
    {
        KalmanTracker tracker = KalmanTracker(bboxes[umd]);
        trackers.push_back(tracker);
    }

    // get trackers' output
    for (auto it = trackers.begin(); it != trackers.end();)
    {
        if (((*it).m_time_since_update < 1) &&
            ((*it).m_hit_streak >= min_hits))
//            ((*it).m_hit_streak >= min_hits || f_num <= min_hits))
        {
            TrackingBox res;
            res.box = (*it).get_state();
            res.id = (*it).m_id + 1;
            res.frame = 0;
            Sort_result.push_back(res);
            it++;
        }
        else
            it++;

        // remove dead tracklet
        if (it != trackers.end() && (*it).m_time_since_update > max_age)
            it = trackers.erase(it);
    }

    //std::cout << "SORT time : " << duration.count() << " ms" << std::endl;

    return Sort_result;
};


void update_dataFrame(int f_num, std::vector<vector<float>> bbox) {
    std::vector<TrackingBox> detData;
    for (int i = 0; i < bbox.size(); i++) {
        TrackingBox tb;
        tb.frame = f_num + 1;
        tb.box = Rect_<float>(cv::Point_<float>(bbox[i][0], bbox[i][1]), cv::Point_<float>(bbox[i][2], bbox[i][3]));
        detData.push_back(tb);
    }
    detFrameData.push_back(detData);
}

std::vector<TrackingBox> get_first_frame_result(std::vector<cv::Rect> bboxes) {
//	int f_num = detFrameData.size() - 1;
    std::vector<TrackingBox> first_frame_boxes;
    for (unsigned int i = 0; i < bboxes.size(); i++) {
        KalmanTracker trk = KalmanTracker(bboxes[i]);
        trackers.push_back(trk);
    }
    // output the first frame detections
    for (unsigned int id = 0; id < bboxes.size(); id++) {
        TrackingBox tb;
        tb.frame = 0;
        tb.id = id;
        tb.box = bboxes[id];
        first_frame_boxes.push_back(tb);
    }
    return first_frame_boxes;
}

std::vector<TrackingBox> SORT(std::vector<cv::Rect> bboxes) {
    std::vector<TrackingBox> frameTrackingResult;
//	update_dataFrame(fi, bbox);

    if (trackers.size() == 0) {
        frameTrackingResult = get_first_frame_result(bboxes);
    }
    else {
        std::vector<cv::Rect_<float>> predictedBoxes = get_predictions();
        MatchItems matched_items = Sort_match(bboxes, predictedBoxes);
        frameTrackingResult = update_trackers(bboxes, matched_items);
    }
    return frameTrackingResult;
}

void vis_id(const std::vector<TrackingBox>& tracking_result, cv::Mat& frm){
    for (auto tb : tracking_result) {
        string num = std::to_string(tb.id);
        cv::Point pt = cv::Point(tb.box.x, tb.box.y);
        cv::putText(frm, num, pt, cv::FONT_HERSHEY_DUPLEX, 2.0, cv::Scalar(0, 255, 255), 1);
    }
}
