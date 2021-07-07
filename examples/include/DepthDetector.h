#pragma once
#define LINE1 1.0
#define LINE2 0.71
#define LINE3 0.57
#define LINE4 0.50
#define LINE5 0.47

#define LINE1_X 4.07
#define LINE2_X 6.23
#define LINE3_X 9.32
#define LINE4_X 12.41

#define POOL_LENGTH_IN_METER 10.0
#define POOL_WIDTH_IN_METER 16.0
#include <opencv2/opencv.hpp>


// swimming pool coordinates structure
struct pool_coord {
    double x;
    double y;
};

// bounding box coordinates structure
struct b_box_coord{
	double x1;
	double y1;
	double x2;
	double y2;
};

b_box_coord normalize_bbox(cv::Rect rect, double col, double row);

//Return pool coordinates when inputing bounding box data
pool_coord return_drowning_normalized_xy(const b_box_coord& input);


//Drawing Line of the swimming pool on the 2D plane
void drawLine(cv::Mat image, std::vector<double> pos);

cv::Mat create_image_plane(std::vector<double> lane_pos, double col , double row);

void visualize2DLocation(cv::Mat image, std::vector<pool_coord> coords);

int return_area_id(const pool_coord& pool);