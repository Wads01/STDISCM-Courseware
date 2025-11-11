#pragma once

#include <string>

class Consumer {
public:
	Consumer(std::string output_dir);
	void operator()();

private:
	std::string output_dir_;
};
