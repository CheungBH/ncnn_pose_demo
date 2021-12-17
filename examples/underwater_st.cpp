// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include <opencv2/core/core.hpp>
#include "AIProcessor.h"

#if CV_MAJOR_VERSION >= 33
#include <opencv2/videoio/videoio.hpp>
#endif

#include <vector>
#include <stdio.h>

#define NCNN_PROFILING

#ifdef NCNN_PROFILING
#include "benchmark.h"
#endif

std::vector<std::string> video_vector = {".mp4", ".avi", "MOV", "MP4"};
std::vector<std::string> image_vector = {".jpg", ".png"};
int min_folder_length = 5;


int main(int argc, char** argv)
{

    ConsoleVariableSystem::get()->readFromCfgFile("../../user.cfg");

    double program_begin = ncnn::get_current_time();

    cv::Mat frame;
    cv::VideoCapture cap;
    cv::VideoWriter outputVideo;

    const char* devicepath;

    int is_streaming = 0;
    int wait_key = 0;
    int cnt = 0;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s [v4l input device or image]\n", argv[0]);
        return -1;
    }

    devicepath = argv[1];
    std::string path_str(devicepath);

#ifdef NCNN_PROFILING
    double t_load_start = ncnn::get_current_time();
#endif

#ifdef NCNN_PROFILING
    double t_load_end = ncnn::get_current_time();
    fprintf(stdout, "NCNN Init time %.02lfms\n", t_load_end - t_load_start);
#endif

    if (find_kws(path_str, image_vector))
    {
        frame = cv::imread(argv[1], 1);
        if (frame.empty())
        {
            fprintf(stderr, "Failed to read image %s.\n", argv[1]);
            return -1;
        }
    }
    else
    {
        if (find_kws(path_str, video_vector)){
            cap.open(devicepath);
        }else if (path_str.size() < min_folder_length){
            cap.open(std::stoi(path_str));
        }else{
            fprintf(stderr, "Failed to open %s", devicepath);
        }

        if (argc > 2){
            int coder = cv::VideoWriter::fourcc('M','J','P','G');
            cv::Size S = cv::Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH),
                                  (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT));
            outputVideo.open(argv[2], coder, 15.0, S, true);
        }

        if (!cap.isOpened())
        {
            fprintf(stderr, "Failed to open %s", devicepath);
            return -1;
        }

        cap >> frame;

        if (frame.empty())
        {
        	
            fprintf(stderr, "Failed to read from device %s.\n", devicepath);
            return -1;
        }

        is_streaming = 1;
        wait_key = 1;
    }

    AIProcessor::init();
    while (true)
    {
        cnt ++;
        if (is_streaming)
        {
#ifdef NCNN_PROFILING
            double t_capture_start = ncnn::get_current_time();
#endif

            cap >> frame;

#ifdef NCNN_PROFILING
            double t_capture_end = ncnn::get_current_time();
            fprintf(stdout, "NCNN OpenCV capture time %.02lfms\n", t_capture_end - t_capture_start);
#endif
            if (frame.empty())
            {
                double program_end = ncnn::get_current_time();
                fprintf(stdout, "The whole program costs %.02lfms\n", program_end - program_begin);
                fprintf(stderr, "OpenCV Failed to Capture from device %s\n", devicepath);
                return -1;
            }
        }
        cv::Mat drown_frame = AIProcessor::process(frame, cnt);
        if (is_streaming){
//            cv::imshow("img_cnt", im_cnt);
        }
        cv::imshow("pose", drown_frame);
        cv::waitKey(wait_key);

        if (!is_streaming)
        {   //If it is a still image, exit!
            if (argc > 2){
                cv::imwrite(argv[2], drown_frame);
            }
            return 0;
        }
        else{
            if (argc > 2){
                outputVideo << drown_frame;
            }
        }
    }

}
