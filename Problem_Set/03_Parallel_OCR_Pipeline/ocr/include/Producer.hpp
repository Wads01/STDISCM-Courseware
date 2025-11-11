#pragma once

#include <string>

class Producer {
public:
	Producer(std::string dataset_dir);
	void operator()();

private:
	std::string dataset_dir_;
};
