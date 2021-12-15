#include "RegionProcessor.h"


RegionProcessor::RegionProcessor(double w, double h, double w_num, double h_num, bool write) {
	height = h; width = w;
	height_num = h_num; width_num = w_num;
	region_cnt = h_num * w_num;
	h_interval = h / h_num; w_interval = w / w_num;

	//region_idx declaration
	for (double i = 0; i < w_num; i++) {
		for (double j = 0; j < h_num; j++) {
			region_idx.push_back({ i,j });
		}
	}

	//REGIONS declaration
	for (int i = 0; i < region_cnt; i++) {
		std::pair<double, double> location = region_idx[i];
		SRBox box {location, h_interval, w_interval};
		REGIONS.insert({ location, box });
	}


	if_write = write;

	keep_case = {};
	update_case = {};
	empty_case = {};
	alarm_case = {};

};

void RegionProcessor::clear()
{
	keep_case = {};
	update_case = {};
	empty_case = {};
	alarm_case = {};
}

std::pair<double, double> RegionProcessor::locate(std::pair<double, double> pt)
{
	double x = std::floor(pt.first / w_interval);
	double y = std::floor(pt.second / h_interval);
	std::pair<double, double> location = { x,y };

	return location;
}

std::vector< std::pair<double, double> > RegionProcessor::locate_cover(std::pair<double, double> pt_tl, std::pair<double, double> pt_br) {

	std::pair<double, double> tl = locate(pt_tl);
	std::pair<double, double> br = locate(pt_br);
	std::vector< std::pair<double, double> > cover;

	for (double i = (tl.first + 1); i < br.first; i++) {
		for (double j = (tl.second + 1); j < br.second; j++) {

			cover.push_back({ i,j });

		}
	}
	return cover;
}

std::vector< std::pair<double, double> > RegionProcessor::locate_occupy(std::pair<double, double> pt_tl, std::pair<double, double> pt_br) {

	std::pair<double, double> tl = locate(pt_tl);
	std::pair<double, double> br = locate(pt_br);
	std::vector< std::pair<double, double> > occupy;

	for (double i = tl.first; i < (br.first + 1); i++) {
		for (double j = tl.second; j < (br.second + 1); j++) {

			occupy.push_back({ i,j });

		}
	}
	return occupy;
}

std::vector< std::pair<double, double> > RegionProcessor::center_region(std::vector<std::vector<double>> boxes) {

	std::vector< std::pair<double, double>> center_regions;

	for (auto &box : boxes) // access by reference to avoid copying
	{
		std::pair<double, double> center = cal_center_point(box);
		center_regions.push_back(locate(center));
	}

	return center_regions;

}


std::vector< std::pair<double, double> > RegionProcessor::cover_region(std::vector<std::vector<double>> boxes) {

	std::vector< std::pair<double, double>> cover_regions;

	for (auto &box : boxes) // access by reference to avoid copying
	{
		std::pair<double, double> pt_tl(box[0], box[1]), pt_br(box[2], box[3]);
		std::vector< std::pair<double, double>> covers = RegionProcessor::locate_cover(pt_tl, pt_br);
		for (auto &cover : covers) {
			cover_regions.push_back(cover);
		}
	}
	return cover_regions;
}

std::vector< std::pair<double, double> > RegionProcessor::occupy_region(std::vector<std::vector<double>> boxes) {

	std::vector< std::pair<double, double>> occupy_regions;

	for (auto &box : boxes) // access by reference to avoid copying
	{
		std::pair<double, double> pt_tl(box[0], box[1]), pt_br(box[2], box[3]);
		std::vector< std::pair<double, double>> occupies = RegionProcessor::locate_occupy(pt_tl, pt_br);
		for (auto &occupy : occupies) {
			occupy_regions.push_back(occupy);
		}
	}
	return occupy_regions;
}

void RegionProcessor::update_region(std::vector<std::vector<std::pair<double, double>>> region_ls) {
	for (auto & update : region_ls[0]) {
		RegionProcessor::REGIONS.at(update).update(1);
	}
	for (auto & keep : region_ls[1]) {
		RegionProcessor::REGIONS.at(keep).update(0);
	}
	for (auto & empty : region_ls[2]) {
		RegionProcessor::REGIONS.at(empty).update(-1);
	}
}


cv::Mat RegionProcessor::draw_cnt_map(cv::Mat img) {
	int region_num = RegionProcessor::region_cnt;
	for (auto & REGION : RegionProcessor::REGIONS)
	{
		cv::Point locate(REGION.second.center.first, REGION.second.center.second);
		std::string time = std::to_string(REGION.second.exist);
		cv::putText(img, time, locate, cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0, 255, 255), 1);
	}
	return img;
}

std::vector<std::vector<std::pair<double, double>>> RegionProcessor::box2region(std::vector< std::pair<double, double>> centers, std::vector< std::pair<double, double>> covers, std::vector< std::pair<double, double>> occupies) {
	keep_case = {};
	update_case = {};
	empty_case = {};

	for (auto &r_idx : region_idx) {
		if (is_element_in_vector(centers, r_idx)) {
			RegionProcessor::update_case.push_back(r_idx);
		}
		else if (is_element_in_vector(covers, r_idx)) {
			RegionProcessor::update_case.push_back(r_idx);
		}
		else if (is_element_in_vector(occupies, r_idx)) {
			RegionProcessor::keep_case.push_back(r_idx);
		}
		else {
			RegionProcessor::empty_case.push_back(r_idx);
		}
	}
	std::vector<std::vector<std::pair<double, double>>> result;
	result.push_back(update_case);
	result.push_back(keep_case);
	result.push_back(empty_case);
	return result;
}

std::vector<std::vector<double>> RegionProcessor::rect2vec(std::vector<cv::Rect> boxes) {
	std::vector<std::vector<double>> converted;
	for (auto &box : boxes) {
		double x1 = box.x;
		double y1 = box.y;
		double x2 = box.x + box.width;
		double y2 = box.y + box.height;
		std::vector<double> box_vec = { x1, y1, x2, y2 };
		converted.push_back(box_vec);
	}
	return converted;
}
/*
void RegionProcessor::draw_boundary(cv::Mat image) {
	for (auto &center_region : center_regions) {
		SRBox box1 = RP.REGIONS.at(center_region); // Find the SRBox that contains the center of the Bounding Box
		cv::Scalar color = (255, 255, 255);
		box1.visualize_srbox(image, color);
	}
}
*/
std::vector<std::pair<double, double>> RegionProcessor::trigger_alarm() {
	std::vector<std::pair<double, double>> alarm_idx;
	for (auto & region_idx : RegionProcessor::region_idx) {
		if (RegionProcessor::REGIONS.at(region_idx).exist > 100) {
			alarm_idx.push_back(region_idx);
		}
	}
	return alarm_idx;
}


std::vector<std::vector<std::pair<double, double>>> RegionProcessor::get_condition(std::vector<cv::Rect> rect_boxes) {
	std::vector<std::vector<double>> boxes = RegionProcessor::rect2vec(rect_boxes);
	std::vector<std::pair<double, double>> center_regions = RegionProcessor::center_region(boxes);
	std::vector<std::pair<double, double>> cover_regions = RegionProcessor::cover_region(boxes);
	std::vector<std::pair<double, double>> occupy_regions = RegionProcessor::occupy_region(boxes);

	RegionProcessor::clear();
	std::vector<std::vector<std::pair<double, double>>> result = RegionProcessor::box2region(center_regions, cover_regions, occupy_regions);
	return result;
}

std::vector<int> RegionProcessor::region_range(cv::Rect box) {
	std::vector<int> region_location;
	std::pair<double, double> tl = { box.x, box.y };
	std::pair<double, double> br = { box.x + box.width, box.y + box.height };
	std::pair<double, double> tl_region = RegionProcessor::locate(tl);
	std::pair<double, double> br_region = RegionProcessor::locate(br);
	region_location.push_back(int(tl_region.first));
	region_location.push_back(int(tl_region.second));
	region_location.push_back(int(br_region.first));
	region_location.push_back(int(br_region.second));
	return region_location;
}

std::vector<int> RegionProcessor::get_alarming_id(std::vector<TrackingBox> tracked_boxes) {
	std::vector<int> warnings;
	std::vector<std::pair<double, double>> alarm_cases = RegionProcessor::trigger_alarm();
	for (auto &tracked_box : tracked_boxes) {
		std::vector<int> box_region = RegionProcessor::region_range(tracked_box.box);
		//int x1, y1, x2, y2 = box_region[0], box_region[1], box_region[2], box_region[3];
		for (auto &alarm_idx : alarm_cases) {
			if (alarm_idx.first >= box_region[0] && alarm_idx.first <= box_region[2] && alarm_idx.second >= box_region[1] && alarm_idx.second <= box_region[3]) {
				warnings.push_back(tracked_box.id);
			}
		}
	}
	//std::cout << "WARNING IDX : " << warnings << std::endl;
	return warnings;
}

