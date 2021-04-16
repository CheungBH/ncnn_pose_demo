#ifndef IMG_TNS_H
#define IMG_TNS_H
#include <opencv2/opencv.hpp>
#include <vector>

cv::Mat& yolo_img(cv::Mat& img, cv::Mat& padded_img, double resize_ratio, bool greyScale);
int sppe_boundary(int n, int lower, int upper);
cv::Rect check_box_size(cv::Mat img, cv::Rect rect);
cv::Mat& sppe_img(cv::Mat& img, const cv::Rect& rect, int img_width, int img_height);
cv::Mat& padded_sppe_img(cv::Mat& img, cv::Mat& padded_img, const cv::Rect& rect, int img_width, int img_height);


#endif
