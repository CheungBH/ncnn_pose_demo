//
// Created by sean on 2021/4/8.
//
#include "yolov.h"
//#include "Hungarian.h"
#include "utils.h"

#include <algorithm>
#include <chrono>

#include "ConsoleVariableSystem.h"

//yolo begin

AutoInt YOLO_TENSOR_W(416, "YOLO_TENSOR_W", "YOLO_TENSOR_W", ConsoleVariableFlag::NONE);
AutoInt YOLO_TENSOR_H(416, "YOLO_TENSOR_H", "YOLO_TENSOR_H", ConsoleVariableFlag::NONE);
AutoInt YOLO_TENSOR_C(3, "YOLO_TENSOR_C", "YOLO_TENSOR_C", ConsoleVariableFlag::NONE);
AutoInt YOLO_TENSOR_N(1, "YOLO_TENSOR_N", "YOLO_TENSOR_N", ConsoleVariableFlag::NONE);

int boundary(int n, int lower, int upper)
{
    return (n > upper ? upper : (n < lower ? lower : n));
}

int yolov::init_yolov4(ncnn::Net* yolov4, int* target_size)
{
    /* --> Set the params you need for the ncnn inference <-- */

    yolov4->opt.num_threads = 2; //You need to compile with libgomp for multi thread support
    yolov4->opt.openmp_blocktime = 0;
    
    yolov4->opt.use_vulkan_compute = true; //You need to compile with libvulkan for gpu support

    yolov4->opt.use_winograd_convolution = true;
    yolov4->opt.use_sgemm_convolution = true;
    yolov4->opt.use_fp16_packed = true;
    yolov4->opt.use_fp16_storage = true;
    yolov4->opt.use_fp16_arithmetic = true;
    yolov4->opt.use_packing_layout = true;
    yolov4->opt.use_shader_pack8 = false;
    yolov4->opt.use_image_storage = false;

    /* --> End of setting params <-- */
    int ret = 0;

    // original pretrained model from https://github.com/AlexeyAB/darknet
    // the ncnn model https://drive.google.com/drive/folders/1YzILvh0SKQPS_lrb33dmGNq7aVTKPWS0?usp=sharing
    // the ncnn model https://github.com/nihui/ncnn-assets/tree/master/models
#ifndef YOLOV4_TINY
//    const char* yolov4_param = "/home/sean/Desktop/ncnn/build/auto_examples/model_yolo/5_ALL-prune_0.95_keep_0.1_10_shortcut/ncnn_opt-fp16.param";
//    const char* yolov4_model = "/home/sean/Desktop/ncnn/build/auto_examples/model_yolo/5_ALL-prune_0.95_keep_0.1_10_shortcut/ncnn_opt-fp16.bin";
    const char* yolov4_param = "../../weights/yolo_rgb/ncnn.param";
    const char* yolov4_model = "../../weights/yolo_rgb/ncnn.bin";
    *target_size = 416;
#else
    const char* yolov4_param = "yolov4-opt.param";
    const char* yolov4_model = "yolov4-opt.bin";
    *target_size = 608;
#endif

    ret = yolov4->load_param(yolov4_param);
    if (ret != 0)
    {
        return ret;
    }

    ret = yolov4->load_model(yolov4_model);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int yolov::detect_yolov4(cv::Mat bgr, std::vector<Object>& objects, int target_size, ncnn::Net* yolov4)
{

    // auto start = std::chrono::steady_clock::now();

    int img_w = bgr.cols;
    int img_h = bgr.rows;

    double resize_ratio;
    cv::Scalar grey_value(128 , 128, 128);
    cv::Mat gray_img(target_size, target_size, CV_8UC3, grey_value);

    if(img_w > img_h)
    {
        resize_ratio = (double)target_size/(double)img_w ;
    }
    else
    {
        resize_ratio = (double)target_size/(double)img_h ;
    }
    double padded_x, padded_y;
    double new_w  = img_w * resize_ratio;
    double new_h = img_h * resize_ratio;
    cv::Size new_sz(new_w,new_h);

    if(img_w > img_h)
    {
        padded_x = 0;
        padded_y = (0.5)*((double)416 - new_h);
    }
    else
    {
        padded_x = (0.5)*((double)416 - new_w);
        padded_y = 0;
    }
    cv::resize(bgr, bgr, new_sz);

    bgr.copyTo(gray_img(cv::Rect(padded_x, padded_y, new_w, new_h)));
//    cv::imshow("test", gray_img);

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(gray_img.data, ncnn::Mat::PIXEL_BGR2RGB, gray_img.cols, gray_img.rows, target_size, target_size);

    const float mean_vals[3] = {0, 0, 0};
    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    in.substract_mean_normalize(mean_vals, norm_vals);

    // auto end = std::chrono::steady_clock::now();

    // std::chrono::duration<double> duration = end - start;
    // std::cout << "[Detector] yolo resize time: " << duration.count() << "s\n";

    // start = std::chrono::steady_clock::now();
    ncnn::Extractor ex = yolov4->create_extractor();

    ex.input("data", in);

    ncnn::Mat out;
    ex.extract("output", out);

    // end = std::chrono::steady_clock::now();

    // duration = end - start;
    // std::cout << "[Detector] yolo inference time: " << duration.count() << "s\n";

    objects.clear();
    for (int i = 0; i < out.h; i++)
    {
        const float* values = out.row(i);

        double xmin = ( values[2]* target_size + (-(double)0.5*((double)target_size - (resize_ratio * img_w) )) ) * ((double)img_w / (double)new_w);
        double ymin = ( values[3]* target_size + (-(double)0.5*((double)target_size - (resize_ratio * img_h) )) ) * ((double)img_h / (double)new_h);
        double xmax = ( values[4]* target_size + (-(double)0.5*((double)target_size - (resize_ratio * img_w) )) ) * ((double)img_w / (double)new_w);
        double ymax = ( values[5]* target_size + (-(double)0.5*((double)target_size - (resize_ratio * img_h) )) ) * ((double)img_h / (double)new_h);
//        double width = xmax - xmin;
//        double height = ymax - ymin;

        double temp[4] = {xmin, ymin, xmax, ymax};
        for (int j = 0; j < 4; j++)
        {
            temp[j] = boundary(temp[j], 0, (j % 2 != 1 ? img_w - 1 : img_h - 1));
        }

        Object object;
        object.label = values[0];
        object.prob = values[1];
        object.rect.x = temp[0];
        object.rect.y = temp[1];
        object.rect.width = temp[2]-temp[0];
        object.rect.height = temp[3]-temp[1];
        if(object.rect.width != 0 && object.rect.height !=0 && values[0] == 1)
        {
            objects.push_back(object);
        }
    }

    return 0;
}

int yolov::detect_padded_yolov4(const cv::Mat& bgr, std::vector<Object>& objects, int target_size, double resize_ratio, double orig_w, double orig_h, ncnn::Net* yolov4)
{
    int img_w = bgr.cols;
    int img_h = bgr.rows;
//    double orig_w = orig_w;
//    double orig_h = orig_h;
    double resized_w = resize_ratio * orig_w;
    double resized_h = resize_ratio * orig_h;

    // auto start = std::chrono::steady_clock::now();

//    ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR2RGB, bgr.cols, bgr.rows, target_size, target_size);
    ncnn::Mat in = ncnn::Mat::from_pixels(bgr.data, ncnn::Mat::PIXEL_BGR2RGB, bgr.cols, bgr.rows);
    const float mean_vals[3] = {0, 0, 0};
    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    in.substract_mean_normalize(mean_vals, norm_vals);

    // auto end = std::chrono::steady_clock::now();

    // std::chrono::duration<double> duration = end - start;
    // std::cout << "[Detector] yolo resize time: " << duration.count() << "s\n";

    // start = std::chrono::steady_clock::now();
    ncnn::Extractor ex = yolov4->create_extractor();

    ex.input("data", in);

    ncnn::Mat out;
    ex.extract("output", out);

    // end = std::chrono::steady_clock::now();

    // duration = end - start;
    // std::cout << "[Detector] yolo inference time: " << duration.count() << "s\n";

    objects.clear();
    for (int i = 0; i < out.h; i++)
    {
        const float* values = out.row(i);

        double xmin = ( values[2]* img_w + (-(double)0.5*((double)YOLO_TENSOR_W.get() - (resize_ratio * orig_w) )) ) * ((double)orig_w / (double)resized_w);
        double ymin = ( values[3]* img_h + (-(double)0.5*((double)YOLO_TENSOR_H.get() - (resize_ratio * orig_h) )) ) * ((double)orig_h / (double)resized_h);
        double xmax = ( values[4]* img_w + (-(double)0.5*((double)YOLO_TENSOR_W.get() - (resize_ratio * orig_w) )) ) * ((double)orig_w / (double)resized_w);
        double ymax = ( values[5]* img_h + (-(double)0.5*((double)YOLO_TENSOR_H.get() - (resize_ratio * orig_h) )) ) * ((double)orig_h / (double)resized_h);
//        double width = xmax - xmin;
//        double height = ymax - ymin;

        double temp[4] = {xmin, ymin, xmax, ymax};
        for (int j = 0; j < 4; j++)
        {
            temp[j] = boundary(temp[j], 0, (j % 2 != 1 ? orig_w - 1 : orig_h - 1));
        }

        Object object;
        object.label = values[0];
        object.prob = values[1];
        object.rect.x = temp[0];
        object.rect.y = temp[1];
        object.rect.width = temp[2]-temp[0];
        object.rect.height = temp[3]-temp[1];
        if(object.rect.width != 0 && object.rect.height !=0)
        {
            objects.push_back(object);
        }
    }

    return 0;
}

void yolov::draw_objects(cv::Mat& bgr, const std::vector<Object>& objects, int is_streaming)
{
    static const char* class_names[] = {"background", "person", "bicycle",
                                        "car", "motorbike", "aeroplane", "bus", "train", "truck",
                                        "boat", "traffic light", "fire hydrant", "stop sign",
                                        "parking meter", "bench", "bird", "cat", "dog", "horse",
                                        "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
                                        "backpack", "umbrella", "handbag", "tie", "suitcase",
                                        "frisbee", "skis", "snowboard", "sports ball", "kite",
                                        "baseball bat", "baseball glove", "skateboard", "surfboard",
                                        "tennis racket", "bottle", "wine glass", "cup", "fork",
                                        "knife", "spoon", "bowl", "banana", "apple", "sandwich",
                                        "orange", "broccoli", "carrot", "hot dog", "pizza", "donut",
                                        "cake", "chair", "sofa", "pottedplant", "bed", "diningtable",
                                        "toilet", "tvmonitor", "laptop", "mouse", "remote", "keyboard",
                                        "cell phone", "microwave", "oven", "toaster", "sink",
                                        "refrigerator", "book", "clock", "vase", "scissors",
                                        "teddy bear", "hair drier", "toothbrush"
    };

    for (size_t i = 0; i < objects.size(); i++)
    {
        const Object& obj = objects[i];

        // fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
        //         obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);

        cv::rectangle(bgr, obj.rect, cv::Scalar(255, 0, 0));

        char text[256];
        // sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);

        int baseLine = 0;
        cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

        int x = obj.rect.x;
        int y = obj.rect.y - label_size.height - baseLine;
        if (y < 0)
            y = 0;
        if (x + label_size.width > bgr.cols)
            x = bgr.cols - label_size.width;

        cv::rectangle(bgr, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                      cv::Scalar(255, 255, 255), -1);

        cv::putText(bgr, text, cv::Point(x, y + label_size.height),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
    }
}
