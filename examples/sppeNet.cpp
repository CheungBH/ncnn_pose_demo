//
// Created by sean on 2021/4/9.
//

#include "ai.h"

#include "net.h"

#include <algorithm>
#include <chrono>
#include <opencv2/core/core.hpp>
#include <Img_tns.h>
extern int SPPE_TENSOR_H, SPPE_TENSOR_W;

//sPPE begin
void ncnn_ai::cropImageFrom(std::vector<cv::Mat> &target, cv::Mat &src, const std::vector<Object> &obj)
{
    target.clear();
//    printf("Crop Image...\n");

    //...
    cv::Scalar grey_value(128, 128, 128);
    cv::Mat sppe_padded_img(SPPE_TENSOR_H, SPPE_TENSOR_W, CV_8UC3, grey_value);
    cv::Mat padded_temp = sppe_padded_img.clone();

    for(auto itr = obj.begin(); itr != obj.end(); itr++)
    {
        double area = itr->rect.width*itr->rect.height;

        cv::Mat padded = padded_sppe_img(src, padded_temp, itr->rect, itr->rect.width, itr->rect.height);
        target.push_back(padded.clone());
    }

    return;
}

std::vector<KP> ncnn_ai::sppeOne(const cv::Mat &src, const ncnn::Net& sppeNet)
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

void ncnn_ai::draw_pose(const cv::Mat& bgr, const std::vector<KP>& keypoints, int is_streaming, const Object& obj)
{
    // draw bone
    static const int joint_pairs[16][2] = {
            {0, 1}, {1, 3}, {0, 2}, {2, 4}, {5, 6}, {5, 7}, {7, 9}, {6, 8}, {8, 10}, {5, 11}, {6, 12}, {11, 12}, {11, 13}, {12, 14}, {13, 15}, {14, 16}
    };

    for (int i = 0; i < 16; i++)
    {
        KP p1 = keypoints[joint_pairs[i][0]];
        KP p2 = keypoints[joint_pairs[i][1]];

        p1.p.x *= obj.rect.width / SPPE_TENSOR_W;
        p1.p.x += obj.rect.x;
        p1.p.y *= obj.rect.height / SPPE_TENSOR_H;
        p1.p.y += obj.rect.y;

        p2.p.x *= obj.rect.width / SPPE_TENSOR_W;
        p2.p.x += obj.rect.x;
        p2.p.y *= obj.rect.height / SPPE_TENSOR_H;
        p2.p.y += obj.rect.y;

        if (p1.prob < 0.04f || p2.prob < 0.04f)
            continue;
        cv::line(bgr, p1.p, p2.p, cv::Scalar(255, 0, 0), 2);
    }

    // draw joint
    for (size_t i = 0; i < keypoints.size(); i++)
    {
        KP keypoint = keypoints[i];

        keypoint.p.x *= obj.rect.width / SPPE_TENSOR_W;
        keypoint.p.x += obj.rect.x;
        keypoint.p.y *= obj.rect.height / SPPE_TENSOR_H;
        keypoint.p.y += obj.rect.y;

        // fprintf(stderr, "%.2f %.2f = %.5f\n", keypoint.p.x, keypoint.p.y, keypoint.prob);

        if (keypoint.prob < 0.2f)
            continue;

        cv::circle(bgr, keypoint.p, 3, cv::Scalar(0, 255, 0), -1);
    }

    if(is_streaming)
    {
        cv::waitKey(10);
    }
    else
    {
        cv::waitKey(0);
    }
}
//sPPE end
