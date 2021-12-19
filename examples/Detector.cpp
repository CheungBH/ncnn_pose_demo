//
// Created by hkuit164 on 16/12/2021.
//

#ifndef NCNN_DETECTOR_CPP
#define NCNN_DETECTOR_CPP

#endif //NCNN_DETECTOR_CPP
#include "Detector.h"
#include <math.h>

int input_size;
std::string detector_type;
int loaded_det;
std::string select_type;
float scaling_ratio;

void Detector::init_detector(ncnn::Net *net){
    input_size = ConsoleVariableSystem::get()->getIntVariableCurrentByHash("detectorSize");
    detector_type = ConsoleVariableSystem::get()->getStringVariableCurrentByHash("detectorType");
    select_type = ConsoleVariableSystem::get()->getStringVariableCurrentByHash("selectType");
    scaling_ratio = ConsoleVariableSystem::get()->getFloatVariableCurrentByHash("scalingRatio");
    if (detector_type == "yolo"){
        loaded_det = yolov::init_yolov4(net);
    }else if (detector_type == "nanodet"){
        loaded_det = nanodet::init_nanodet(net);
    }
}

std::vector<Object> Detector::select_objects(std::vector<Object> objects, int image_height, int image_width){
    if (select_type == "largest"){
        return Detector::select_largest(objects);
    }else if (select_type == "center"){
        return Detector::select_center(objects, image_height, image_width);
    }else{
        return objects;
    }
}

std::vector<Object> Detector::select_largest(std::vector<Object> objects){
    std::vector<Object> selected_objects;
    Object selected_object;
    float max_size = 0;
    for (auto& object : objects) {
        float box_size = object.rect.width * object.rect.height;
        if (box_size > max_size){
            selected_object = object;
            max_size = box_size;
        }
    }
    selected_objects.push_back(selected_object);
    return selected_objects;
}

float Detector::calculate_distance(cv::Point p1, cv::Point p2){
    return sqrt(pow(abs(p1.x - p2.x),2) + pow(abs(p1.y - p2.y),2));
}

std::vector<Object> Detector::select_center(std::vector<Object> objects, int image_height, int image_width){
    std::vector<Object> selected_objects;
    Object selected_object;
    float min_distance = image_height * image_width;
    cv::Point image_center;
    image_center.x = image_width / 2;
    image_center.y = image_height / 2;

    for (auto& object : objects) {
        cv::Point box_center;
        box_center.x = object.rect.x + object.rect.width/2;
        box_center.y = object.rect.y + object.rect.height/2;
        float box_image_dist = calculate_distance(image_center, box_center);
        if (box_image_dist < min_distance){
            selected_object = object;
            min_distance = box_image_dist;
        }
    }
    selected_objects.push_back(selected_object);
    return selected_objects;
}

std::vector<Object> Detector::scale(std::vector<Object> objects, int image_height, int image_width){
    std::vector<Object> scaled_objects;
    for (auto& object : objects) {
        float scale_width = scaling_ratio * object.rect.width;
        float scale_height = scaling_ratio * object.rect.height;
        object.rect.x -= scale_width;
        object.rect.y -= scale_height;
        object.rect.width += 2 * scale_width;
        object.rect.height += 2 * scale_height;
        if (object.rect.x < 0) {object.rect.x = 0;}
        if (object.rect.y < 0) {object.rect.y = 0;}
        if (object.rect.x + object.rect.width > image_width) {object.rect.width = image_width - object.rect.x - 1;}
        if (object.rect.y + object.rect.height > image_height) {object.rect.height = image_height - object.rect.y - 1;}
        scaled_objects.push_back(object);
    }
    return scaled_objects;
}


void Detector::detect(cv::Mat &bgr, std::vector <Object> &objects, ncnn::Net *net) {
    if (loaded_det == 0) {
        if (detector_type == "yolo") {
            yolov::detect_yolov4(bgr, objects, input_size, net); //Create an extractor and run detection
        } else if (detector_type == "nanodet") {
            nanodet::detect_nanodet(net, bgr, objects, input_size);
        }
        int image_height = bgr.rows;
        int image_width = bgr.cols;
        objects = Detector::select_objects(objects, image_height, image_width);
        objects = Detector::scale(objects, image_height, image_width);
        Detector::draw_objects(bgr, objects); //Draw detection results on opencv image
    } else {
        cv::Rect_<float> whole_image_box;
        whole_image_box.x = 0;
        whole_image_box.y = 0;
        whole_image_box.width = bgr.cols - 1;
        whole_image_box.height = bgr.rows - 1;
        Object obj;
        obj.rect = whole_image_box;
        obj.prob = 1;
        obj.label = 0;
        objects.push_back(obj);
    }
}

void Detector::draw_objects(cv::Mat& bgr, const std::vector<Object>& objects){
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

        fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
                obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);

        cv::rectangle(bgr, obj.rect, cv::Scalar(255, 0, 0));

        char text[256];
        sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);

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
