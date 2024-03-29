//
// Created by sean on 2021/4/9.
//
#include "sppeNet.h"
#include "Img_tns.h"
#include "ConsoleVariableSystem.h"

#include <algorithm>
#include <chrono>

AutoInt SPPE_TENSOR_W(256, "SPPE_TENSOR_W", "SPPE_TENSOR_W", ConsoleVariableFlag::NONE);
AutoInt SPPE_TENSOR_H(256, "SPPE_TENSOR_H", "SPPE_TENSOR_H", ConsoleVariableFlag::NONE);
AutoInt SPPE_TENSOR_C(3, "SPPE_TENSOR_C", "SPPE_TENSOR_C", ConsoleVariableFlag::NONE);
AutoInt SPPE_TENSOR_N(1, "SPPE_TENSOR_N", "SPPE_TENSOR_N", ConsoleVariableFlag::NONE);

void sppeNet::cropImageOriginal(std::vector<cv::Mat> &target, const cv::Mat &src, const std::vector<Object> &obj)
{
    target.clear();

    //...
    for(auto itr = obj.begin(); itr != obj.end(); itr++)
    {
        target.push_back(src(itr->rect).clone());
    }

    return;
}

std::vector<KP> sppeNet::sppeOneAll(const cv::Mat &src, const ncnn::Net &sppeNet, const Object& obj) {

    std::vector<KP> target;
    cv::Mat img_tmp = src.clone();
    cv::Scalar grey_value(128, 128, 128);
    cv::Mat sppe_padded_img(SPPE_TENSOR_H.get(), SPPE_TENSOR_W.get(), CV_8UC3, grey_value);

    int original_w = src.cols, original_h = src.rows;
    double resize_ratio = 1;
    double resize_ratio_1 = (double)SPPE_TENSOR_W.get()/original_w;
    double resize_ratio_2 = (double)SPPE_TENSOR_H.get()/original_h;

    double resize_ratio_final = resize_ratio_1 < resize_ratio_2 ? resize_ratio_1 : resize_ratio_2;
    resize_ratio = resize_ratio_final;

    double new_w = (double)original_w * resize_ratio;
    double new_h = (double)original_h * resize_ratio;
    double padded_x ;
    double padded_y ;
    if(original_w > original_h)
    {
        padded_x = 0;
        padded_y = (0.5)*((double)SPPE_TENSOR_H.get() - new_h);
        //std::cout << "w: " << img.cols << " h: " << img.rows << " resize_ratio: " << resize_ratio <<std::endl;
    }
    else
    {
        padded_x = (0.5)*((double)SPPE_TENSOR_W.get() - new_w);
        padded_y = 0;
        //std::cout << "w: " << img.cols << " h: " << img.rows << " resize_ratio: " << resize_ratio <<std::endl;
    }
    cv::Size new_sz(new_w,new_h);
    cv::resize(img_tmp, img_tmp, new_sz);
    cv::imshow("resized", img_tmp);
    img_tmp.copyTo(sppe_padded_img(cv::Rect(padded_x, padded_y, new_w, new_h)));

    cv::imshow("padded", sppe_padded_img);
//    cv::waitKey(0);


    auto start = std::chrono::steady_clock::now();


    ncnn::Mat in = ncnn::Mat::from_pixels_resize(sppe_padded_img.data, ncnn::Mat::PIXEL_BGR2RGB, SPPE_TENSOR_W.get(),
                                                SPPE_TENSOR_H.get(), SPPE_TENSOR_W.get(), SPPE_TENSOR_H.get());

    const float mean_vals[3] = {0.485f*255.f, 0.456f*255.f, 0.406f*255.f};
    const float norm_vals[3] = {1/0.229f/255.f, 1/0.224f/255.f, 1/0.225f/255.f};
//    const float mean_vals[3] = {0.406f*255.f, 0.456f*255.f, 0.485f*255.f};
//    const float norm_vals[3] = {1 / 255.f/0.225f, 1 / 255.f/0.224f, 1 / 255.f/0.229f};
    in.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor ex = sppeNet.create_extractor();

    ex.input("input.1", in);

    ncnn::Mat out;
    ex.extract("342", out);

    int out_w = out.w, out_h = out.h;

    // resolve point from heatmap
    target.clear();
    for (int p = 0; p < out.c; p++)
    {
        const ncnn::Mat m = out.channel(p);
        std::cout << typeid(m).name()<<std::endl;
        float max_prob = 0.f;
        int max_x = 0;
        int max_y = 0;
        for (int y = 0; y < out.h; y++)
        {
            const float* ptr = m.row(y);
            for (int x = 0; x < out.w; x++)
            {
                float prob = ptr[x];
                if (prob > max_prob)
                {
                    max_prob = prob;
                    max_x = x;
                    max_y = y;
                }
            }
        }

        KP keypoint;
        float coord_x = (float) ((float) max_x / (float) out_w * (float) SPPE_TENSOR_W.get() - (float )padded_x) / (float) resize_ratio + obj.rect.x;
        float coord_y = (float) ((float) max_y / (float) out_h * (float) SPPE_TENSOR_H.get() - (float )padded_y) / (float) resize_ratio + obj.rect.y;
        keypoint.p = cv::Point2f(coord_x, coord_y);
//        keypoint.p = cv::Point2f(max_x * w / (float)out.w, max_y * h / (float)out.h);
        keypoint.prob = max_prob;

        target.push_back(keypoint);
    }

    return target;
}

//sPPE begin
void sppeNet::cropImageFrom(std::vector<cv::Mat> &target, cv::Mat &src, const std::vector<Object> &obj)
{
    target.clear();
//    printf("Crop Image...\n");

    //...
    cv::Scalar grey_value(128, 128, 128);
    cv::Mat sppe_padded_img(SPPE_TENSOR_H.get(), SPPE_TENSOR_W.get(), CV_8UC3, grey_value);
    cv::Mat padded_temp = sppe_padded_img.clone();

    for(auto itr = obj.begin(); itr != obj.end(); itr++)
    {
        double area = itr->rect.width*itr->rect.height;

        cv::Mat padded = padded_sppe_img(src, padded_temp, itr->rect, itr->rect.width, itr->rect.height);
        cv::imshow("padded_sppe", padded);
        target.push_back(padded.clone());
    }

    return;
}

std::vector<KP> sppeNet::sppeOne(const cv::Mat &src, const ncnn::Net& sppeNet)
{
    std::vector<KP> target;

    int w = src.cols, h = src.rows;

    auto start = std::chrono::steady_clock::now();

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(src.data, ncnn::Mat::PIXEL_BGR2RGB, w, h, 256, 320);

    const float mean_vals[3] = {0.485f * 255.f, 0.456f * 255.f, 0.406f * 255.f};
    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    in.substract_mean_normalize(mean_vals, norm_vals);

    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> duration = end - start;
    std::cout << "[SPPE] sppeNet resize time: " << duration.count() << "s\n";

    start = std::chrono::steady_clock::now();

    ncnn::Extractor ex = sppeNet.create_extractor();

    ex.input("input.1", in);

    ncnn::Mat out;
    ex.extract("1123", out);

    end = std::chrono::steady_clock::now();

    duration = end - start;
    std::cout << "[SPPE] sppeNet inference time: " << duration.count() << "s\n";


    for (int p = 0; p < out.c; p++)
    {
        const ncnn::Mat m = out.channel(p);

        float max_prob = 0.f;
        int max_x = 0;
        int max_y = 0;
        for (int y = 0; y < out.h; y++)
        {
            const float* ptr = m.row(y);
            for (int x = 0; x < out.w; x++)
            {
                float prob = ptr[x];
                if (prob > max_prob)
                {
                    max_prob = prob;
                    max_x = x;
                    max_y = y;
                }
            }
        }

        KP keypoint;
        keypoint.p = cv::Point2f(max_x * w / (float)out.w, max_y * h / (float)out.h);
        keypoint.prob = max_prob;

        target.push_back(keypoint);
    }

    return target;
}

//void sppeNet::draw_pose(const cv::Mat& bgr, const std::vector<KP>& keypoints)
//{
//    // draw bone
//    int kps = keypoints.size();
//    int joints_num;
//
//    if (kps == 17){
//        joints_num = 16;
//    }else{
//        joints_num = 11;
//    }
//    int joint_pairs[joints_num][2];
//
//    if (kps == 17){
//        static const int joint_pairs[16][2] = {
//                {0, 1}, {1, 3}, {0, 2}, {2, 4}, {5, 6}, {5, 7}, {7, 9}, {6, 8}, {8, 10}, {5, 11}, {6, 12}, {11, 12}, {11, 13}, {12, 14}, {13, 15}, {14, 16}
//        };
//    }else if (kps == 13){
//        static const int joint_pairs[11][2] = {
//                {1, 2}, {2, 4}, {4, 6}, {1, 3}, {3, 5}, {1, 7}, {2, 8}, {7, 9}, {8, 10}, {9, 11}, {10, 12}
//        };
//    }else{
//        static const int joint_pairs[16][2] = {
//                {0, 1}, {1, 3}, {0, 2}, {2, 4}, {5, 6}, {5, 7}, {7, 9}, {6, 8}, {8, 10}, {5, 11}, {6, 12}, {11, 12}, {11, 13}, {12, 14}, {13, 15}, {14, 16}
//        };
//    }
//
//    for (int i = 0; i < joints_num; i++)
//    {
//        KP p1 = keypoints[joint_pairs[i][0]];
//        KP p2 = keypoints[joint_pairs[i][1]];
//
//
//        if (p1.prob < 0.04f || p2.prob < 0.04f)
//            continue;
//        cv::line(bgr, p1.p, p2.p, cv::Scalar(255, 0, 0), 2);
//    }
//
//    // draw joint
//    for (size_t i = 0; i < keypoints.size(); i++)
//    {
//        KP keypoint = keypoints[i];
//
////        keypoint.p.x *= obj.rect.width / SPPE_TENSOR_W.get();
////        keypoint.p.x += obj.rect.x;
////        keypoint.p.y *= obj.rect.height / SPPE_TENSOR_H.get();
////        keypoint.p.y += obj.rect.y;
////        keypoint.p.x += obj.rect.x;
////        keypoint.p.y += obj.rect.y;
//
//        // fprintf(stderr, "%.2f %.2f = %.5f\n", keypoint.p.x, keypoint.p.y, keypoint.prob);
//
//        if (keypoint.prob < 0.2f)
//            continue;
//
//        cv::circle(bgr, keypoint.p, 3, cv::Scalar(0, 255, 0), -1);
//    }
//
//}

void sppeNet::draw_pose(const cv::Mat& bgr, const std::vector<KP>& keypoints)
{
    // draw bone
//    static const int joint_pairs[16][2] = {
//            {0, 1}, {1, 3}, {0, 2}, {2, 4}, {5, 6}, {5, 7}, {7, 9}, {6, 8}, {8, 10}, {5, 11}, {6, 12}, {11, 12}, {11, 13}, {12, 14}, {13, 15}, {14, 16}
//    };
    static const int joint_pairs[12][2] = {
            {1, 2}, {1, 3}, {3, 5}, {2, 4}, {4, 6}, {1, 7}, {2, 8}, {7, 9}, {9, 11}, {8, 10}, {10, 12}, {7, 8}
    };

    for (int i = 0; i < 12; i++)
    {
        KP p1 = keypoints[joint_pairs[i][0]];
        KP p2 = keypoints[joint_pairs[i][1]];


        if (p1.prob < 0.04f || p2.prob < 0.04f)
            continue;
        cv::line(bgr, p1.p, p2.p, cv::Scalar(255, 0, 0), 2);
    }

    // draw joint
    for (size_t i = 0; i < keypoints.size(); i++)
    {
        KP keypoint = keypoints[i];

//        keypoint.p.x *= obj.rect.width / SPPE_TENSOR_W.get();
//        keypoint.p.x += obj.rect.x;
//        keypoint.p.y *= obj.rect.height / SPPE_TENSOR_H.get();
//        keypoint.p.y += obj.rect.y;
//        keypoint.p.x += obj.rect.x;
//        keypoint.p.y += obj.rect.y;

        // fprintf(stderr, "%.2f %.2f = %.5f\n", keypoint.p.x, keypoint.p.y, keypoint.prob);

        if (keypoint.prob < 0.2f)
            continue;

        cv::circle(bgr, keypoint.p, 3, cv::Scalar(0, 255, 0), -1);
    }

}

//void sppeNet::draw_pose(const cv::Mat& bgr, const std::vector<KP>& keypoints, int is_streaming, const Object& obj)
//{
//    // draw bone
//    static const int joint_pairs[16][2] = {
//            {0, 1}, {1, 3}, {0, 2}, {2, 4}, {5, 6}, {5, 7}, {7, 9}, {6, 8}, {8, 10}, {5, 11}, {6, 12}, {11, 12}, {11, 13}, {12, 14}, {13, 15}, {14, 16}
//    };
//
//    for (int i = 0; i < 16; i++)
//    {
//        KP p1 = keypoints[joint_pairs[i][0]];
//        KP p2 = keypoints[joint_pairs[i][1]];
//
//        p1.p.x *= obj.rect.width / SPPE_TENSOR_W.get();
//        p1.p.x += obj.rect.x;
//        p1.p.y *= obj.rect.height / SPPE_TENSOR_H.get();
//        p1.p.y += obj.rect.y;
//
//        p2.p.x *= obj.rect.width / SPPE_TENSOR_W.get();
//        p2.p.x += obj.rect.x;
//        p2.p.y *= obj.rect.height / SPPE_TENSOR_H.get();
//        p2.p.y += obj.rect.y;
//
//        if (p1.prob < 0.04f || p2.prob < 0.04f)
//            continue;
//        cv::line(bgr, p1.p, p2.p, cv::Scalar(255, 0, 0), 2);
//    }
//
//    // draw joint
//    for (size_t i = 0; i < keypoints.size(); i++)
//    {
//        KP keypoint = keypoints[i];
//
//        keypoint.p.x *= obj.rect.width / SPPE_TENSOR_W.get();
//        keypoint.p.x += obj.rect.x;
//        keypoint.p.y *= obj.rect.height / SPPE_TENSOR_H.get();
//        keypoint.p.y += obj.rect.y;
//
//        // fprintf(stderr, "%.2f %.2f = %.5f\n", keypoint.p.x, keypoint.p.y, keypoint.prob);
//
//        if (keypoint.prob < 0.2f)
//            continue;
//
//        cv::circle(bgr, keypoint.p, 3, cv::Scalar(0, 255, 0), -1);
//    }
//
//    if(is_streaming)
//    {
//        cv::waitKey(10);
//    }
//    else
//    {
//        cv::waitKey(0);
//    }
//}
//sPPE end
