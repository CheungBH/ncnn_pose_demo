//
// Created by sean on 2021/4/9.
//

#include "ai.h"
#include "net.h"

#include <algorithm>
#include <chrono>
#include <opencv2/core/core.hpp>

//CNN begin
std::vector<float> ncnn_ai::cnn(const cv::Mat &src, const ncnn::Net& cnnNet)
{
    std::vector<float> target;

    auto start = std::chrono::steady_clock::now();

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(src.data, ncnn::Mat::PIXEL_BGR, src.cols, src.rows, 224, 224);

    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    in.substract_mean_normalize(0, norm_vals);

    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> duration = end - start;
    std::cout << "[CNN] cnnNet resize time: " << duration.count() << "s\n";

    start = std::chrono::steady_clock::now();
    ncnn::Extractor ex = cnnNet.create_extractor();

    ex.input("input.1", in);

    ncnn::Mat out;
    ex.extract("191", out);

    end = std::chrono::steady_clock::now();

    duration = end - start;
    std::cout << "[CNN] cnnNet inference time: " << duration.count() << "s\n";

    {
        ncnn::Layer* softmax = ncnn::create_layer("Softmax");

        ncnn::ParamDict pd;
        softmax->load_param(pd);

        softmax->forward_inplace(out, cnnNet.opt);

        delete softmax;
    }

    out = out.reshape(out.w * out.h * out.c);

    for(int i = 0; i < out.w; i++)
    {
        target.push_back(out[i]);
    }

    return target;
}

int ncnn_ai::print_topk(const std::vector<float>& cls_scores, int topk)
{
    // partial sort topk with index
    int size = cls_scores.size();
    std::vector<std::pair<float, int> > vec;
    vec.resize(size);
    for (int i = 0; i < size; i++)
    {
        vec[i] = std::make_pair(cls_scores[i], i);
    }

    std::partial_sort(vec.begin(), vec.begin() + topk, vec.end(),
                      std::greater<std::pair<float, int> >());

    // print topk and score
    for (int i = 0; i < topk; i++)
    {
        float score = vec[i].first;
        int index = vec[i].second;
        fprintf(stderr, "%d = %f\n", index, score);
    }

    return 0;
}
//CNN end