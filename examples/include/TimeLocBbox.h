#pragma once
#include <vector>
#include <string>
#include <stdio.h>
#include <time.h>
#include <sstream>
#include "DepthDetector.h"

template<class T>
std::string toString(const T &value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

struct time_loc_bbox 
{   
    int cam_id;
    int area_id;
	std::string time;
	pool_coord loc;
    b_box_coord bbox;
};

//Doc: https://en.cppreference.com/w/cpp/chrono/c/strftime
std::string currentDateTime();
std::string tlbToString(time_loc_bbox tlb);

