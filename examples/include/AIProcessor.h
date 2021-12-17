//
// Created by hkuit164 on 15/12/2021.
//

#ifndef NCNN_AIPROCESSOR_H
#define NCNN_AIPROCESSOR_H

#endif //NCNN_AIPROCESSOR_H

#include "Detector.h"
#include "cnnNet.h"
#include "sppeNet.h"
#include "Tracker.h"
#include "DrownAnalysis.h"
#include "RegionProcessor.h"
#include "utils.h"
#include "ConsoleVariableSystem.h"
#include "net.h"

namespace AIProcessor{
    cv::Mat process(cv::Mat frame);
    void init();
};