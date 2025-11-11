#pragma once

#include <string>
#include <memory>

class OCRPipeline;

class Consumer {
public:
	Consumer(std::string output_dir);
	void operator()();

private:
	std::string output_dir_;
	std::unique_ptr<OCRPipeline> pipeline_;
};
