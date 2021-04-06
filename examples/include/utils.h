#pragma once
#include <iostream>
#include <utility>
#include <vector>

//Image
#include <opencv2/opencv.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

class Utils {

public:
	//Input the bounding box (xmin,ymin,xmax,ymax) and return the a pair of (x,y) coord of center
	std::pair<double, double> cal_center_point(std::vector<double> box);
	bool is_int_element_in_vector(std::vector<int> v, int element);
	bool is_element_in_vector(std::vector< std::pair<double, double>> v, std::pair<double, double> element);
	std::vector<std::vector<float>> Rect2vf(std::vector<cv::Rect> rect_boxes);
};