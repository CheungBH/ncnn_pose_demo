//
// Created by sean on 2021/4/9.
//

#include "ai.h"

#include "net.h"
#include "RegionProcessor.h"
#include "Hungarian.h"
#include "utils.h"

#include <algorithm>
#include <chrono>
#include <opencv2/core/core.hpp>

//sPPE begin
void ncnn_ai::cropImageFrom(std::vector<cv::Mat> &target, const cv::Mat &src, const std::vector<Object> &obj)
{
    target.clear();
    printf("Crop Image...\n");

    //...
    for(auto itr = obj.begin(); itr != obj.end(); itr++)
    {
        target.push_back(src(itr->rect).clone());
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
    std::cout << "sppeNet resize time: " << duration.count() << "s\n";

    start = std::chrono::steady_clock::now();

    ncnn::Extractor ex = sppeNet.create_extractor();

    ex.input("input.1", in);

    ncnn::Mat out;
    ex.extract("1123", out);

    end = std::chrono::steady_clock::now();

    duration = end - start;
    std::cout << "sppeNet extract time: " << duration.count() << "s\n";


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

void ncnn_ai::draw_pose(const cv::Mat& bgr, const std::vector<KP>& keypoints, int is_streaming)
{
    cv::Mat image = bgr.clone();

    // draw bone
    static const int joint_pairs[16][2] = {
            {0, 1}, {1, 3}, {0, 2}, {2, 4}, {5, 6}, {5, 7}, {7, 9}, {6, 8}, {8, 10}, {5, 11}, {6, 12}, {11, 12}, {11, 13}, {12, 14}, {13, 15}, {14, 16}
    };

    for (int i = 0; i < 16; i++)
    {
        const KP& p1 = keypoints[joint_pairs[i][0]];
        const KP& p2 = keypoints[joint_pairs[i][1]];

        if (p1.prob < 0.04f || p2.prob < 0.04f)
            continue;

        cv::line(image, p1.p, p2.p, cv::Scalar(255, 0, 0), 2);
    }

    // draw joint
    for (size_t i = 0; i < keypoints.size(); i++)
    {
        const KP& keypoint = keypoints[i];

        // fprintf(stderr, "%.2f %.2f = %.5f\n", keypoint.p.x, keypoint.p.y, keypoint.prob);

        if (keypoint.prob < 0.2f)
            continue;

        cv::circle(image, keypoint.p, 3, cv::Scalar(0, 255, 0), -1);
    }

    cv::imshow("pose", image);
    if(is_streaming)
    {
        cv::waitKey(1);
    }
    else
    {
        cv::waitKey(0);
    }
}
//sPPE end
