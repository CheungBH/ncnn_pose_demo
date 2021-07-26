#include "DepthDetector.h"
#include "ConsoleVariableSystem.h"

#include <opencv2/opencv.hpp>

AutoFloat LINE1(1.0, "LINE1", "LINE1", ConsoleVariableFlag::NONE);
AutoFloat LINE2(0.71, "LINE2", "LINE2", ConsoleVariableFlag::NONE);
AutoFloat LINE3(0.57, "LINE3", "LINE3", ConsoleVariableFlag::NONE);
AutoFloat LINE4(0.50, "LINE4", "LINE4", ConsoleVariableFlag::NONE);
AutoFloat LINE5(0.47, "LINE5", "LINE5", ConsoleVariableFlag::NONE);

AutoFloat LINE1_X(4.07, "LINE1_X", "LINE1_X", ConsoleVariableFlag::NONE);
AutoFloat LINE2_X(6.23, "LINE2_X", "LINE2_X", ConsoleVariableFlag::NONE);
AutoFloat LINE3_X(9.32, "LINE3_X", "LINE3_X", ConsoleVariableFlag::NONE);
AutoFloat LINE4_X(12.41, "LINE4_X", "LINE4_X", ConsoleVariableFlag::NONE);

AutoFloat POOL_LENGTH_IN_METER(10.0, "POOL_LENGTH_IN_METER", "POOL_LENGTH_IN_METER", ConsoleVariableFlag::NONE);
AutoFloat POOL_WIDTH_IN_METER(16.0, "POOL_WIDTH_IN_METER", "POOL_WIDTH_IN_METER", ConsoleVariableFlag::NONE);

b_box_coord normalize_bbox(const cv::Rect& rect, double col, double row)
{
    double x1 = rect.x; double y1 = rect.y;
    double x2 = x1+rect.width; double y2 = y1+rect.height;
    b_box_coord b_box_normalized {
            x1/double(col),
            y1/double(row),
            x2/double(col),
            y2/double(row)
    };
    // std::cout << x1 << "," << y1 << "," << x2 << "," << y2 <<std::endl;
    return b_box_normalized;
}

//Return pool coordinates when inputing bounding box data
pool_coord return_drowning_normalized_xy(const b_box_coord& input) {
    pool_coord pool{ -1, -1 };

    // find y2 range
    if (LINE2.get() < input.y2 && input.y2 <= LINE1.get()) {
        pool.y = 7.6 + (((input.y2 - LINE2.get()) / (LINE1.get() - LINE2.get())) * 2.0);

        //x: 3m
        pool.x = (POOL_WIDTH_IN_METER.get() - LINE1_X.get()) / 2.0 + LINE1_X.get() * (input.x1 + input.x2) / 2.0;
    }
    else if (LINE3.get() < input.y2 && input.y2 <= LINE2.get()) {
        pool.y = 6 + (((input.y2 - LINE3.get()) / (LINE2.get() - LINE3.get())) * 2);

        // x: 5m
        pool.x = (POOL_WIDTH_IN_METER.get() - LINE2_X.get()) / 2.0 + LINE2_X.get() * (input.x1 + input.x2) / 2.0;
    }
    else if (LINE4.get() < input.y2 && input.y2 <= LINE3.get()) {
        pool.y = 4 + (((input.y2 - LINE4.get()) / (LINE3.get() - LINE4.get())) * 2);

        // x: 7m
        pool.x = (POOL_WIDTH_IN_METER.get() - LINE3_X.get()) / 2.0 + LINE3_X.get() * (input.x1 + input.x2) / 2;
    }
    else if (LINE5.get() <= input.y2 && input.y2 <= LINE4.get()) {
        pool.y = 2 + (((input.y2 - LINE5.get()) / (LINE4.get() - LINE5.get())) * 2);

        // x: 9m
        pool.x = (POOL_WIDTH_IN_METER.get() - LINE4_X.get()) / 2.0 + LINE4_X.get() * (input.x1 + input.x2) / 2;
    }
    else{
        //throw "Out of Range!";
    }
    // std::cout << "pool.x : " << pool.x << "pool.y : " << pool.y  << std::endl;

    pool.x /= POOL_WIDTH_IN_METER.get();
    pool.y /= POOL_LENGTH_IN_METER.get();

    // std::cout << "norm pool.x : " << pool.x << "norm pool.y : " << pool.y  << std::endl;



    return std::move(pool);
}


//Drawing Line of the swimming pool on the 2D plane
void drawLine(cv::Mat image, std::vector<double> pos){

    for( auto& po : pos) {
        cv::Scalar colorLine(0, 255, 0); // Green
        int lineThickness = 2;
        cv::Point pt1, pt2;
        pt1.x = 0 ; pt1.y = double(image.rows)*po;
        pt2.x = image.cols ; pt2.y = double(image.rows)*po;
        cv::line(image, pt1, pt2, colorLine, lineThickness);
    }

    return;

}

cv::Mat create_image_plane(std::vector<double> lane_pos, double col , double row)
{
    cv::Mat image_plane(col / 4.0, row / 4.0, CV_8UC3, cv::Scalar(0, 0, 0));
    //Visualize the swimming lane
    // std::vector<double> pos2 = { 0.2 , 0.4, 0.6, 0.8};
    drawLine(image_plane, lane_pos);
    //Visualize the covering angle of the camera
    int lineThickness = 2;
    cv::Point pt1, pt2;
    cv::Scalar colorLine(0, 255, 0); // Green
    pt1.x = 0 ; pt1.y = double(image_plane.rows) * 0.2;
    pt2.x = double(image_plane.cols) * 0.33; pt2.y = image_plane.rows;
    cv::line(image_plane, pt1, pt2, colorLine, lineThickness);
    pt1.x = image_plane.cols ; pt1.y = double(image_plane.rows) * 0.2;
    pt2.x = double(image_plane.cols) * 0.66 ; pt2.y = image_plane.rows;
    cv::line(image_plane, pt1, pt2, colorLine, lineThickness);

    return image_plane;
}

void visualize2DLocation(cv::Mat image, std::vector<pool_coord> coords){
    std::pair<double, double> planeLoc = { 0, 0 };
    for(auto& coord : coords)
    {
        if(coord.x != -1)
        {
            planeLoc.first = (coord.x ) * image.cols;
            planeLoc.second = (coord.y ) * image.rows;
            std::cout << "planeLoc.first: "<< planeLoc.first << " planeLoc.second: "<< planeLoc.second << std::endl;
            cv::Point center(planeLoc.first, planeLoc.second);
            cv::circle(image, center, 3, cv::Scalar(0, 255, 0), -1);
        }
        else
        {
            std::cout << "coord.x: "<< coord.x << " coord.y: "<< coord.y << std::endl;
        }

    }

}

int return_area_id(const pool_coord& pool){
    if (pool.y >= 0.6){
        if (pool.x >= 0.75){
            return 8;
        }
        else if (pool.x >= 0.5){
            return 7;
        }
        else if (pool.x >= 0.25){
            return 6;
        }
        else{
            return 5;
        }
    }
    else if (pool.y >= 0.2){
        if (pool.x >= 0.75){
            return 4;
        }
        else if (pool.x >= 0.5){
            return 3;
        }
        else if (pool.x >= 0.25){
            return 2;
        }
        else{
            return 1;
        }
    }
    return -1;
}