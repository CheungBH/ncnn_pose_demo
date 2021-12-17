#include "AIProcessor.h"

#define SCREEN_W 960
#define SCREEN_H 540

int loaded_cnn, loaded_sppe;
ncnn::Net detector, estimator, classifier;
std::vector<std::vector<KP>> skeletons;
std::vector<std::vector<float>> predictions;
double image_height_pixel = SCREEN_H;
double image_width_pixel = SCREEN_W;
cv::Mat im_raw(image_height_pixel, image_width_pixel, CV_8UC3, cv::Scalar(0, 0, 0));

double w_num = 10;
double h_num = 10;
RegionProcessor RP {image_width_pixel, image_height_pixel, w_num, h_num, false};
DrownAnalysis analysis;


void AIProcessor::init(){

    Detector::init_detector(&detector);
    loaded_cnn = cnnNet::init_CNN(&classifier);
    loaded_sppe = sppeNet::init_sppe(&estimator);
//    List list;
    analysis = DrownAnalysis{};
    cv::Mat im_cnt;
}

cv::Mat AIProcessor::process(cv::Mat frame, int frame_cnt){
    cv::Mat drown_frame = frame.clone();
    std::vector<cv::Mat> imgs;
    std::vector<cv::Rect> b_boxes;
    std::vector<Object> objects;

    Detector::detect(frame, objects, &detector);
    for (const auto& object : objects) {
        b_boxes.push_back(object.rect);
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

    if (frame_cnt > 1){
        //// The time of the first is too large; Needed to be solved
        auto drown_start =  std::chrono::steady_clock::now();
        analysis.update(frameTrackingResult, drown_frame.rows);
        // analysis.print();
        drown_frame = analysis.visualize(drown_frame);
        std::vector<cv::Rect> drown_boxes = analysis.get_red_box();
        auto drown_duration = duration_cast<milliseconds>(std::chrono::steady_clock::now() - drown_start);
        std::cout << "[Drown] Time taken for drown analysis " << drown_duration.count() << " ms" << std::endl;

    }

    skeletons.clear();
    predictions.clear();

    auto crop_start = std::chrono::steady_clock::now();
    sppeNet::cropImageOriginal(imgs, frame, objects);
    auto crop_duration = duration_cast<milliseconds>(std::chrono::steady_clock::now() - crop_start);
    std::cout << "[Crop] Time taken for cropping box " << crop_duration.count() << " ms" << std::endl;

    int i = 0;
    for(auto itr = imgs.begin(); itr != imgs.end(); itr++)
    {
        double area = itr->size[0]*itr->size[1];
        if(area > 10)
        {
            if (loaded_sppe){
                skeletons.push_back(sppeNet::sppeOneAll(*itr, estimator, objects[i].rect));
                sppeNet::draw_pose(drown_frame, skeletons[itr-imgs.begin()]);
            }
            if (loaded_cnn){
                predictions.push_back(cnnNet::cnn(*itr, classifier));
            }
            // print_topk(predictions[itr-imgs.begin()], 2);
            i++;
        }
    }
    return drown_frame;
}

