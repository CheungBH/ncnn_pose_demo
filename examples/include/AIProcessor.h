//
// Created by hkuit164 on 15/12/2021.
//

#ifndef NCNN_AIPROCESSOR_H
#define NCNN_AIPROCESSOR_H

#endif //NCNN_AIPROCESSOR_H

#include "yolov.h"
#include "sppeNet.h"
#include "cnnNet.h"
#include "Tracker.h"
#include "RegionProcessor.h"
#include "Hungarian.h"
#include "utils.h"
#include "ConsoleVariableSystem.h"

class AIProcessor{
public:
    int loaded_sppe, loaded_det, loaded_cnn;

};