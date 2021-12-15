//
// Created by sean on 2021/4/9.
//
#include "sppeNet.h"


int sppeNet::init_sppe(ncnn::Net* sppeNet){
    static bool is_loaded_sppe = false;
    const char* sppeParam = ConsoleVariableSystem::get()->getStringVariableCurrentByHash("sppeParam");
    const char* sppeModel = ConsoleVariableSystem::get()->getStringVariableCurrentByHash("sppeModel");

    std::string sppe_bin(sppeModel), sppe_param(sppeParam);
    if((sppe_bin.size() < 3) or (sppe_param.size() < 3))
    {
        std::cout<<"Not using pose estimator"<<std::endl;
    }else
    {
        sppeNet->opt.use_vulkan_compute = 1;
        sppeNet->load_param(sppeParam);
        sppeNet->load_model(sppeModel);
        is_loaded_sppe = true;
    }
    return is_loaded_sppe;
}


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

std::vector<KP> sppeNet::sppeOneAll(const cv::Mat &src, const ncnn::Net &sppeNet, const cv::Rect& box) {

    const char* sppeInput = ConsoleVariableSystem::get()->getStringVariableCurrentByHash("sppeInput");
    const char* sppeOutput = ConsoleVariableSystem::get()->getStringVariableCurrentByHash("sppeOutput");

    int SPPE_TENSOR_W  = ConsoleVariableSystem::get()->getIntVariableCurrentByHash("sppeWidth");
    int SPPE_TENSOR_H = ConsoleVariableSystem::get()->getIntVariableCurrentByHash("sppeHeight");

    std::vector<KP> target;

    cv::Mat img_tmp = src.clone();
    cv::Scalar grey_value(128, 128, 128);
    cv::Mat sppe_padded_img(SPPE_TENSOR_H, SPPE_TENSOR_W, CV_8UC3, grey_value);

    int original_w = src.cols, original_h = src.rows;
    double resize_ratio = 1;
    double resize_ratio_1 = (double)SPPE_TENSOR_W/original_w;
    double resize_ratio_2 = (double)SPPE_TENSOR_H/original_h;

    double resize_ratio_final = resize_ratio_1 < resize_ratio_2 ? resize_ratio_1 : resize_ratio_2;
    resize_ratio = resize_ratio_final;

    double new_w = (double)original_w * resize_ratio;
    double new_h = (double)original_h * resize_ratio;
    double padded_x ;
    double padded_y ;
    if(original_w > original_h)
    {
        padded_x = 0;
        padded_y = (0.5)*((double)SPPE_TENSOR_H - new_h);
        //std::cout << "w: " << img.cols << " h: " << img.rows << " resize_ratio: " << resize_ratio <<std::endl;
    }
    else
    {
        padded_x = (0.5)*((double)SPPE_TENSOR_W - new_w);
        padded_y = 0;
        //std::cout << "w: " << img.cols << " h: " << img.rows << " resize_ratio: " << resize_ratio <<std::endl;
    }
    cv::Size new_sz(new_w,new_h);
    cv::resize(img_tmp, img_tmp, new_sz);
//    cv::imshow("resized", img_tmp);
    img_tmp.copyTo(sppe_padded_img(cv::Rect(padded_x, padded_y, new_w, new_h)));

//    cv::imshow("padded", sppe_padded_img);
//    cv::waitKey(0);


    auto start = std::chrono::steady_clock::now();


    ncnn::Mat in = ncnn::Mat::from_pixels_resize(sppe_padded_img.data, ncnn::Mat::PIXEL_BGR2RGB, SPPE_TENSOR_W,
                                                 SPPE_TENSOR_H, SPPE_TENSOR_W, SPPE_TENSOR_H);

    float sppeMean0 = ConsoleVariableSystem::get()->getFloatVariableCurrentByHash("sppeMean[0]");
    float sppeMean1 = ConsoleVariableSystem::get()->getFloatVariableCurrentByHash("sppeMean[1]");
    float sppeMean2 = ConsoleVariableSystem::get()->getFloatVariableCurrentByHash("sppeMean[2]");

    float sppeNorm0 = ConsoleVariableSystem::get()->getFloatVariableCurrentByHash("sppeNorm[0]");
    float sppeNorm1 = ConsoleVariableSystem::get()->getFloatVariableCurrentByHash("sppeNorm[1]");
    float sppeNorm2 = ConsoleVariableSystem::get()->getFloatVariableCurrentByHash("sppeNorm[2]");

    const float mean_vals[3] = { sppeMean0, sppeMean1, sppeMean2 };
    const float norm_vals[3] = { sppeNorm0, sppeNorm1, sppeNorm2 };

    in.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor ex = sppeNet.create_extractor();

    ex.input(sppeInput, in);

    ncnn::Mat out;
    ex.extract(sppeOutput, out);

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
        float coord_x = (float) ((float) max_x / (float) out_w * (float) SPPE_TENSOR_W - (float )padded_x) / (float) resize_ratio + box.x;
        float coord_y = (float) ((float) max_y / (float) out_h * (float) SPPE_TENSOR_H - (float )padded_y) / (float) resize_ratio + box.y;
        keypoint.p = cv::Point2f(coord_x, coord_y);
//        keypoint.p = cv::Point2f(max_x * w / (float)out.w, max_y * h / (float)out.h);
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
////        keypoint.p.x *= obj.rect.width / SPPE_TENSOR_W;
////        keypoint.p.x += obj.rect.x;
////        keypoint.p.y *= obj.rect.height / SPPE_TENSOR_H;
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
    int sppeKpsIdx = ConsoleVariableSystem::get()->getIntVariableCurrentByHash("sppeKpsIdx");

    // draw bone
    static const int joint_16_pairs[16][2] = {
        {0, 1}, {1, 3}, {0, 2}, {2, 4}, {5, 6}, {5, 7}, {7, 9}, {6, 8}, {8, 10}, {5, 11}, {6, 12}, {11, 12}, {11, 13}, {12, 14}, {13, 15}, {14, 16}
    };

    static const int joint_12_pairs[12][2] = {
        {1, 2}, {1, 3}, {3, 5}, {2, 4}, {4, 6}, {1, 7}, {2, 8}, {7, 9}, {9, 11}, {8, 10}, {10, 12}, {7, 8}
    };

    int sppeKps = ConsoleVariableSystem::get()->getIntVariableCurrentByHash("sppeKps");
    float sppeThresh = ConsoleVariableSystem::get()->getFloatVariableCurrentByHash("sppeThresh");
    
    switch(sppeKpsIdx)
    {
        case 12:
            for (int i = 0; i < sppeKps; i++)
            {
                KP p1 = keypoints[joint_12_pairs[i][0]];
                KP p2 = keypoints[joint_12_pairs[i][1]];


                if (p1.prob < sppeThresh || p2.prob < sppeThresh)
                    continue;
                cv::line(bgr, p1.p, p2.p, cv::Scalar(255, 0, 0), 2);
            }

        case 16:
            for (int i = 0; i < sppeKps; i++)
            {
                KP p1 = keypoints[joint_16_pairs[i][0]];
                KP p2 = keypoints[joint_16_pairs[i][1]];


                if (p1.prob < sppeThresh || p2.prob < sppeThresh)
                    continue;
                cv::line(bgr, p1.p, p2.p, cv::Scalar(255, 0, 0), 2);
            }
    }

    // draw joint
    for (size_t i = 0; i < keypoints.size(); i++)
    {
        KP keypoint = keypoints[i];

//        keypoint.p.x *= obj.rect.width / SPPE_TENSOR_W;
//        keypoint.p.x += obj.rect.x;
//        keypoint.p.y *= obj.rect.height / SPPE_TENSOR_H;
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
//        p1.p.x *= obj.rect.width / SPPE_TENSOR_W;
//        p1.p.x += obj.rect.x;
//        p1.p.y *= obj.rect.height / SPPE_TENSOR_H;
//        p1.p.y += obj.rect.y;
//
//        p2.p.x *= obj.rect.width / SPPE_TENSOR_W;
//        p2.p.x += obj.rect.x;
//        p2.p.y *= obj.rect.height / SPPE_TENSOR_H;
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
//        keypoint.p.x *= obj.rect.width / SPPE_TENSOR_W;
//        keypoint.p.x += obj.rect.x;
//        keypoint.p.y *= obj.rect.height / SPPE_TENSOR_H;
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
