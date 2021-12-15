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

#include "yolov.h"
#include "sppeNet.h"
#include "cnnNet.h"
#include "Tracker.h"
#include "RegionProcessor.h"
#include "Hungarian.h"
#include "utils.h"
#include "ConsoleVariableSystem.h"

#include <algorithm>
#include <opencv2/core/core.hpp>

#include <chrono>


#if CV_MAJOR_VERSION >= 33
#include <opencv2/videoio/videoio.hpp>
#endif

#include <vector>
//#include "RegionProcessor.h"
#include "DrownAnalysis.h"
#include "Img_tns.h"

#include <stdio.h>

#define NCNN_PROFILING
#define YOLOV4_TINY //Using yolov4_tiny, if undef, using original yolov4

#ifdef NCNN_PROFILING
#include "benchmark.h"
#endif


using namespace yolov;
using namespace sppeNet;
using namespace cnnNet;

#define SCREEN_W 960
#define SCREEN_H 540
#define YOLO_INPUT 416


std::vector<std::string> video_vector = {".mp4", ".avi", "MOV", "MP4"};
std::vector<std::string> image_vector = {".jpg", ".png"};
int min_folder_length = 5;

bool find_kws(std::string src_string, std::vector<std::string> kws){
    for (int i = 0; i < kws.size(); i++){
        if (src_string.find(kws[i]) != std::string::npos){
            return true;
        }
    }
    return false;
}


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

    double image_height_pixel = SCREEN_H;
    double image_width_pixel = SCREEN_W;

    double w_num = 10;
    double h_num = 10;

    ncnn::Net yolov4;
    int det_loaded = init_yolov4(&yolov4);
    int yolo_size = ConsoleVariableSystem::get()->getIntVariableCurrentByHash("yoloWidth");

    // init cnnNet
    static ncnn::Net cnnNet;
    int loaded_cnn = init_CNN(&cnnNet);

    // init sppe
    static ncnn::Net sppe_Net;
    int loaded_sppe = init_sppe(&sppe_Net);

//    List list;
    RegionProcessor RP {image_width_pixel, image_height_pixel, w_num, h_num, false};
    DrownAnalysis analysis = DrownAnalysis{};

    std::vector<std::vector<KP>> skeletons;
    std::vector<std::vector<float>> predictions;
    cv::Mat im_raw(image_height_pixel, image_width_pixel, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat drown_frame, im_cnt; //for storing each frame and preprocessed frame;

    while (true)
    {
        if (is_streaming)
        {
#ifdef NCNN_PROFILING
            double t_capture_start = ncnn::get_current_time();
#endif

            cap >> frame;
            im_cnt = im_raw.clone();
            drown_frame = frame.clone();


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

        std::vector<cv::Mat> imgs;
        std::vector<cv::Rect> b_boxes;
        std::vector<Object> objects;

        if (det_loaded == 0){
            detect_yolov4(frame, objects, yolo_size, &yolov4); //Create an extractor and run detection
            draw_objects(frame, objects); //Draw detection results on opencv image
            for (const auto& object : objects) {
                b_boxes.push_back(object.rect);
            }
        }else{
            cv::Rect_<float> whole_image_box;
            whole_image_box.x = 0;
            whole_image_box.y = 0;
            whole_image_box.width = frame.cols - 1;
            whole_image_box.height = frame.rows - 1;
            Object obj;
            obj.rect = whole_image_box;
            obj.prob = 1;
            obj.label = 0;
            objects.push_back(obj);
            b_boxes.push_back(whole_image_box);
        }

        auto start_rp = std::chrono::steady_clock::now();
        std::vector<std::vector<std::pair<double, double>>> RP_res = RP.get_condition(b_boxes);
        RP.update_region(RP_res);
//        cv::Mat img_cnt = RP.draw_cnt_map(im_cnt);
        std::chrono::duration<double> RP_duration = std::chrono::steady_clock::now() - start_rp;
        std::cout << "[Region] Time taken for region processor: " << RP_duration.count() << "s\n";

        auto start_sort = std::chrono::steady_clock::now();
//		std::vector<std::vector<float>> untracked_boxes = utils_main.Rect2vf(b_boxes);
        std::vector<TrackingBox> frameTrackingResult = SORT(b_boxes);
        vis_id(frameTrackingResult, frame);
        auto SORT_duration = duration_cast<milliseconds>(std::chrono::steady_clock::now() - start_sort);
        std::cout << "[Sort] Time taken for sort: " << SORT_duration.count() << "s\n";
        std::vector<int> alarm_idx = RP.get_alarming_id(frameTrackingResult);

        auto drown_start =  std::chrono::steady_clock::now();
        analysis.update(frameTrackingResult, drown_frame.rows);
        // analysis.print();
        drown_frame = analysis.visualize(drown_frame);
        std::vector<cv::Rect> drown_boxes = analysis.get_red_box();
        auto drown_duration = duration_cast<milliseconds>(std::chrono::steady_clock::now() - drown_start);
        std::cout << "[Drown] Time taken for drown analysis " << drown_duration.count() << " ms" << std::endl;

        skeletons.clear();
        predictions.clear();

        auto crop_start = std::chrono::steady_clock::now();
//        std::cout << frame.size << std::endl;
//        cropImageFrom(imgs, frame, objects);
        cropImageOriginal(imgs, frame, objects);
        auto crop_duration = duration_cast<milliseconds>(std::chrono::steady_clock::now() - crop_start);
        std::cout << "[Crop] Time taken for cropping box " << crop_duration.count() << " ms" << std::endl;

        int i = 0;
        for(auto itr = imgs.begin(); itr != imgs.end(); itr++)
        {
            double area = itr->size[0]*itr->size[1];
            if(area > 10)
            {
                if (loaded_sppe){
                    skeletons.push_back(sppeOneAll(*itr, sppe_Net, objects[i].rect));
                    draw_pose(drown_frame, skeletons[itr-imgs.begin()]);
                }
                if (loaded_cnn){
                    predictions.push_back(cnn(*itr, cnnNet));
                }
                // print_topk(predictions[itr-imgs.begin()], 2);
                i++;
            }
        }

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
