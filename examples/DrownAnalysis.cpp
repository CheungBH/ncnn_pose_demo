#include "DrownAnalysis.h"
#include <iostream>

bool DrownAnalysis::judge(const cv::Rect_<float>& box_coord) {
	// drown detection
	// if ((box_coord.y + box_coord.height - box_coord.height * RATIO) < LEVEL){
	int y2 = box_coord.y + box_coord.height;
	int y1 = box_coord.y;
	if ((y2 - (y2 - y1) * RATIO) < LEVEL*img_height) {
		return false;
	}
	else {
		return true;
	}
}

void DrownAnalysis::clear() {
	curr_ids = {};
}

void DrownAnalysis::update(const std::vector<TrackingBox>& frameTrackingResults, int height) {
	DrownAnalysis::clear();
	img_height = height;
	id2boxes = frameTrackingResults;
	auto drown_end_time = std::chrono::high_resolution_clock::now();
	int64 duration = duration_cast<milliseconds>(drown_end_time - drown_start_time).count();

	for (const auto& frameTrackingResult : frameTrackingResults) {
		// update cnt
		curr_ids.push_back(frameTrackingResult.id);
		if (!DrownAnalysis::checkEntryExtist(frameTrackingResult.id)) {
			DrownAnalysis::newEntry();
		}
		if (judge(frameTrackingResult.box)) {
			if (id2cnt[frameTrackingResult.id] < MAX_CNT * SCALAR) {
				id2cnt[frameTrackingResult.id] += duration;
			}
			else {
				id2cnt[frameTrackingResult.id] = MAX_CNT * SCALAR;
			}
		}
		else{
			if (id2cnt[frameTrackingResult.id] > 0) {
				id2cnt[frameTrackingResult.id] -= duration;
			}
			else {
				id2cnt[frameTrackingResult.id] = 0;
			}
		}

		// update signal
		if (id2cnt[frameTrackingResult.id] > RED_CNT*SCALAR) {
			//std::cout << "RED" << std::endl;
			id2signal[frameTrackingResult.id] = DrownSignal::RED;
		}
		else if (id2cnt[frameTrackingResult.id] > YELLOW_CNT*SCALAR) {
			//std::cout << "YELLOW" << std::endl;
			id2signal[frameTrackingResult.id] = DrownSignal::YELLOW;
		}
		else {
			//std::cout << "GREEN" << std::endl;
			id2signal[frameTrackingResult.id] = DrownSignal::GREEN;
		}
	}
	drown_start_time = std::chrono::high_resolution_clock::now();
}

void DrownAnalysis::print() {
	for (const auto& id2box : id2boxes) {
		int id = id2box.id;
		std::cout << "id: " << id << " ---> cnt: " << id2cnt[id] << " ---> " << std::endl;
	}
}


void DrownAnalysis::newEntry() {
	id2cnt.push_back(0);
	id2signal.push_back(DrownSignal::GREEN);
}

bool DrownAnalysis::checkEntryExtist(const int& id) {
	if (id < id2cnt.size()) {
		return true;
	}
	else {
		return false;
	}
}


void DrownAnalysis::visualize(cv::Mat& img) {
	cv::line(img, cv::Point(0, LEVEL*img.rows), cv::Point(img.cols, LEVEL*img.rows), cv::Scalar(201, 7, 22), 4);
	for (const auto& id2box : id2boxes) {
		cv::Scalar color = DrownAnalysis::convert_color(id2box.id);
		float box_line = id2box.box.y + (1 - RATIO) * id2box.box.height;
		cv::line(img, cv::Point(id2box.box.x, box_line), cv::Point(id2box.box.x + id2box.box.width, box_line), cv::Scalar(0, 255, 255), 2);
		cv::rectangle(img, id2box.box, color, 2);
		cv::Point pt = cv::Point(id2box.box.x, id2box.box.y);
		cv::putText(img, "id" + std::to_string(id2box.id) + ":" + std::to_string(id2cnt[id2box.id]/ SCALAR) + "s", pt, cv::FONT_HERSHEY_DUPLEX, 1, color, 2);
	}
}

cv::Scalar DrownAnalysis::convert_color(int id) {
	if (id2signal[id] == DrownSignal::RED) {
		return cv::Scalar(0, 0, 255);
	}
	else if (id2signal[id] == DrownSignal::YELLOW)
	{
		return cv::Scalar(0, 255, 255);
	}
	else
	{
		return cv::Scalar(0, 255, 0);
	} 
}

std::vector<int> DrownAnalysis::get_red_id() {
	std::vector<int> alarm_ids;
	for (const auto& id2box : id2boxes) {
		if (id2signal[id2box.id] == DrownSignal::RED) {
			alarm_ids.push_back(id2box.id);
		}
	}
	return alarm_ids;
}

std::vector<cv::Rect> DrownAnalysis::get_red_box() {
	std::vector<cv::Rect> alarm_boxes;
	for (const auto& id2box : id2boxes) {
		if (id2signal[id2box.id] == DrownSignal::RED) {
			alarm_boxes.push_back(id2box.box);
		}
	}
	return alarm_boxes;
}
