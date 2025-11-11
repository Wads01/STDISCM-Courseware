#pragma once

#include <string>

class Producer {
public:
	Producer(std::string dataset_dir, int consumer_count =1);
	void operator()();

private:
	std::string dataset_dir_;
	int consumer_count_ =1;
};
