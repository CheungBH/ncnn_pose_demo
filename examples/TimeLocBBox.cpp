#include "TimeLocBbox.h"


//Doc: https://en.cppreference.com/w/cpp/chrono/c/strftime
std::string currentDateTime()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", &tstruct);

    return buf;
}

std::string tlbToString(time_loc_bbox tlb)
{
    std::string id = toString(tlb.cam_id);
    std::string area_id = toString(tlb.area_id) + ",";
    std::string location = toString(tlb.loc.x) + "," + toString(tlb.loc.y) + "," ;
    std::string boundingbox = toString(tlb.bbox.x1) + "," +toString(tlb.bbox.y1) + "," +toString(tlb.bbox.x2) + "," +toString(tlb.bbox.y2) ;
    std::string output = "{" + id + "@" + tlb.time + ":" + area_id + boundingbox +"}" ;
    return output;
}

