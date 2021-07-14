#pragma once
#include <vector>
#include "TrackingBox.h"
#include <chrono>
#include <time.h>
using namespace std::chrono;

#define YELLOW_CNT 5
#define RED_CNT 8
#define LEVEL 0.3
#define RATIO 0.6
#define MAX_CNT 15
#define SCALAR 1000


enum class DrownSignal {
	RED,
	YELLOW,
	GREEN
};


class DrownAnalysis {
public:
	// default constructor
	DrownAnalysis() {};

	// constructor
	DrownAnalysis(const std::vector<int>& cnt, const std::vector<DrownSignal>& signal)
		: id2cnt(cnt), id2signal(signal)
	{}

	// copy constructor
	DrownAnalysis(const DrownAnalysis& other)
		: id2cnt(other.id2cnt), id2signal(other.id2signal)
	{}

	// copy assignment
	DrownAnalysis& operator=(const DrownAnalysis& other) {
		this->id2cnt = other.id2cnt;
		this->id2signal = other.id2signal;
		return *this;
	}

	// destructor
	~DrownAnalysis() {};

	inline const int& get_cnt(const int& id) { return id2cnt[id]; }
	inline const DrownSignal& get_signal(const int& id) { return id2signal[id]; }

	bool judge(const cv::Rect_<float>& box_coord);
	void clear();
	// bool judge(const cv::Rect_<float>& box_coord);
	void update(const std::vector<TrackingBox>& frameTrackingResult, int height);
	void print();
	void newEntry();
	bool checkEntryExtist(const int& id);

	void visualize(cv::Mat& img);

	cv::Scalar convert_color(int id);

	std::vector<int> get_red_id();

	std::vector<cv::Rect> get_red_box();

private:
	// std::unordered_map<int, int> id2cnt;
	// std::unordered_map<int, DrownSignal> id2cnt;
	std::vector<int> id2cnt;
	std::vector<DrownSignal> id2signal;
	std::vector<int> curr_ids;
	std::vector<TrackingBox> id2boxes;
	int img_height;
	std::chrono::high_resolution_clock::time_point drown_start_time = std::chrono::high_resolution_clock::now();
};