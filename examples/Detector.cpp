//
// Created by hkuit164 on 16/12/2021.
//

#ifndef NCNN_DETECTOR_CPP
#define NCNN_DETECTOR_CPP

#endif //NCNN_DETECTOR_CPP
#include "Detector.h"

int input_size;
std::string detector_type;
int loaded_det;

void Detector::init_detector(ncnn::Net *net){
    input_size = ConsoleVariableSystem::get()->getIntVariableCurrentByHash("detectorSize");
    detector_type = ConsoleVariableSystem::get()->getStringVariableCurrentByHash("detectorType");
    if (detector_type == "yolo"){
        loaded_det = yolov::init_yolov4(net);
    }else if (detector_type == "nanodet"){
        loaded_det = nanodet::init_nanodet(net);
    }
}

void Detector::detect(const cv::Mat &bgr, std::vector <Object> &objects, ncnn::Net *net) {
    if (loaded_det == 0) {
        if (detector_type == "yolo") {
            yolov::detect_yolov4(bgr, objects, input_size, net); //Create an extractor and run detection
        } else if (detector_type == "nanodet") {
            nanodet::detect_nanodet(net, bgr, objects, input_size);
        }
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

cv::Mat Detector::draw_objects(const cv::Mat& bgr, const std::vector<Object>& objects){
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

    cv::Mat image = bgr.clone();

    for (size_t i = 0; i < objects.size(); i++)
    {
        const Object& obj = objects[i];

        fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
                obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);

        cv::rectangle(image, obj.rect, cv::Scalar(255, 0, 0));

        char text[256];
        sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);

        int baseLine = 0;
        cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

        int x = obj.rect.x;
        int y = obj.rect.y - label_size.height - baseLine;
        if (y < 0)
            y = 0;
        if (x + label_size.width > image.cols)
            x = image.cols - label_size.width;

        cv::rectangle(image, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                      cv::Scalar(255, 255, 255), -1);

        cv::putText(image, text, cv::Point(x, y + label_size.height),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
    }

    return image;
}
