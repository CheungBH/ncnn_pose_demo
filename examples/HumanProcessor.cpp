/*
#include "HumanProcessor.h"
#include "Person.h"
//#include "TrackingBox.h"
#include "utils.h"

Utils utils1 = Utils::Utils();

HumanProcessor::HumanProcessor() {
	stored_id = {};
	curr_id = {};
	RD_id = {};
	warning_id = {};
	untracked_id = {};
	PEOPLE = {};
}

void HumanProcessor::clear()
{
	curr_id = {};
	RD_id = {};
	warning_id = {};
	untracked_id = {};
}

void HumanProcessor::update(std::vector<TrackingBox> tracked_boxes) {
	HumanProcessor::clear();
	for (auto &t_box : tracked_boxes) {
		int id = t_box.id;
		HumanProcessor::curr_id.push_back(id);
		if (utils1.is_int_element_in_vector(HumanProcessor::stored_id, id)) {
			Person new_person = Person::Person(t_box);
			PEOPLE.insert(id, new_person);
		}
		else {
			HumanProcessor::PEOPLE.at(id).update(t_box);
			HumanProcessor::stored_id.push_back(id);
		}
	}
	for (auto &idx : stored_id) {
		if (utils1.is_int_element_in_vector(HumanProcessor::curr_id, idx)) {
		}
		else {
			HumanProcessor::untracked_id.push_back(idx);
		}
	}
	int a = 1;
}
*/







