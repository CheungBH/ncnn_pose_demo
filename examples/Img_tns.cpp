#include "Img_tns.h"
#include "Timer.h"

cv::Mat& yolo_img(cv::Mat& img, cv::Mat& padded_img, double resize_ratio, bool greyScale = false)
{
    double new_w  = (double)img.cols * resize_ratio;
    double new_h = (double)img.rows * resize_ratio;
    double padded_x ;
    double padded_y ;
    if(img.cols > img.rows)
    {
        padded_x = 0;
        padded_y = (0.5)*((double)416 - new_h);
        //std::cout << "w: " << img.cols << " h: " << img.rows << " resize_ratio: " << resize_ratio <<std::endl;
    }
    else
    {
        padded_x = (0.5)*((double)416 - new_w);
        padded_y = 0;
        //std::cout << "w: " << img.cols << " h: " << img.rows << " resize_ratio: " << resize_ratio <<std::endl;
    }

    cv::Size new_sz(new_w,new_h);
    cv::Scalar grey_value( 128, 128, 128);
    //cv::Mat padded_img(416, 416, CV_8UC3, grey_value); 	//Create an image with grey background

    try
    {
        //Letter box
        cv::resize(img, img, new_sz);
        //Put the resized image inside the grey image
        cv::imshow("Img", img);
        cv::imshow("padded_img", padded_img);
        cv::imshow("Region of Interest", padded_img(cv::Rect(padded_x, padded_y, img.cols, img.rows)));
//        cv::waitKey(0);
        img.copyTo(padded_img(cv::Rect(padded_x, padded_y, img.cols, img.rows)));

        if (greyScale){
            cv::cvtColor(padded_img, padded_img, cv::COLOR_BGR2GRAY);
//            cv::cvtColor(padded_img, padded_img, cv::COLOR_GRAY2RGB);
        }
        else{
//            cv::cvtColor(padded_img, padded_img, cv::COLOR_BGR2RGB);
        }
//        padded_img.convertTo(padded_img, CV_32FC3, 1.0f / 255.0f);
        cv::resize(img, img, cv::Size(416, 416));
        img = padded_img.clone();

        return img;
    }
    catch(cv::Exception e)
    {
        std::cerr << e.msg << std::endl;
    }
    std::cout << "Finished" << std::endl;
    return img;
}
int sppe_boundary(int n, int lower, int upper)
{
    return (n > upper ? upper : (n < lower ? lower : n));
}
cv::Rect check_box_size(cv::Mat img, cv::Rect rect) {
    if ((rect.x + rect.width) > img.cols) {
        rect.width = img.cols - rect.x - 1;
    }
    if ((rect.y + rect.height) > img.rows) {
        rect.height = img.rows - rect.y - 1;
    }
    return rect;
}
cv::Mat& sppe_img(cv::Mat& img, const cv::Rect& rect, int img_width, int img_height)
{
    //std::cout << "ROI x: " << rect.x << " y: " << rect.y << " w: " << rect.width << " h: " << rect.height << std::endl;
    int w ;
    int h ;
    w = sppe_boundary(rect.x+rect.width, 0, img_width) - rect.x;
    w = sppe_boundary(w, 0, img_width);
    h = sppe_boundary(rect.y+rect.height, 0, img_height) - rect.y;
    h = sppe_boundary(h, 0, img_height);
    cv::Rect new_rect(rect.x, rect.y, w, h);

    img = img(new_rect);
    //cv::cvtColor(img, img, CV_BGR2RGB);
    cv::resize(img, img, cv::Size(256, 320));
    img.convertTo(img, CV_32FC3, 1.0f / 255.0f);

    return img;
}
cv::Mat& padded_sppe_img(cv::Mat& img, cv::Mat& padded_img, const cv::Rect& rect, int img_width, int img_height)
{
//    Timer timer;
    int w ;
    int h ;
    w = sppe_boundary(rect.x+rect.width, 0, img.cols) - rect.x;
    h = sppe_boundary(rect.y+rect.height, 0, img.rows) - rect.y;
    cv::Rect new_rect(rect.x, rect.y, w, h);
    std::cout << "rect x: " << rect.x << " rect y: " << rect.y << " w: " << w << " h: " << h << std::endl;
    std::cout << "img: " << img.size << std::endl;
    cv::Mat croppedimg = img(new_rect);

    double resize_ratio = 1;
    double resize_ratio_1 = (double)256.0/(double)croppedimg.cols;
    double resize_ratio_2 = (double)320.0/(double)croppedimg.rows;

    double resize_ratio_final = resize_ratio_1 < resize_ratio_2 ? resize_ratio_1 : resize_ratio_2;
    resize_ratio = resize_ratio_final;
//    if(croppedimg.cols > croppedimg.rows){resize_ratio = (double)256.0/(double)croppedimg.cols ;}
//    else{resize_ratio = (double)320.0/(double)croppedimg.rows ;}

    double new_w  = (double)croppedimg.cols * resize_ratio;
    double new_h = (double)croppedimg.rows * resize_ratio;
    double padded_x ;
    double padded_y ;
    if(croppedimg.cols > croppedimg.rows)
    {
        padded_x = 0;
        padded_y = (0.5)*((double)320.0 - new_h);
        //std::cout << "w: " << img.cols << " h: " << img.rows << " resize_ratio: " << resize_ratio <<std::endl;
    }
    else
    {
        padded_x = (0.5)*((double)256.0 - new_w);
        padded_y = 0;
        //std::cout << "w: " << img.cols << " h: " << img.rows << " resize_ratio: " << resize_ratio <<std::endl;
    }
    cv::Size new_sz(new_w,new_h);
    try
    {
        //Letter box
        cv::resize(croppedimg, croppedimg, new_sz);
        //Put the resized image inside the grey image
        croppedimg.copyTo(padded_img(cv::Rect(padded_x, padded_y, croppedimg.cols, croppedimg.rows)));
//        padded_img.convertTo(padded_img, CV_32FC3, 1.0f / 255.0f);
//        cv::resize(croppedimg, croppedimg, cv::Size(256, 320));
//        croppedimg = padded_img.clone();
//        std::cout << "[IMGTNS::PADDEDSPPEIMG]COMPLETED TIME: " << timer.report() <<  std::endl;
        return padded_img;
    }
    catch(cv::Exception e)
    {
        std::cout << " x "  << padded_x <<  " y "  << padded_y << " w "  << croppedimg.cols << " h "  << croppedimg.rows << std::endl;
        std::cerr << e.msg << std::endl;
    }
//    std::cout << "[IMGTNS::PADDEDSPPEIMG]COMPLETED TIME: " << timer.report() <<  std::endl;
    return padded_img;
}
