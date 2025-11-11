#pragma once

#include <string>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include <semaphore>

#include <opencv2/opencv.hpp>

// Shared item used by producer/consumer
struct ImageItem {
	cv::Mat mat;
	std::string filename;
	bool sentinel = false;
};

// Shared queue and synchronization objects (definitions in OCRPipeline.cpp)
extern std::queue<std::shared_ptr<ImageItem>> image_queue;
extern std::mutex queue_mutex;
extern std::counting_semaphore<1024> items_sem;
extern std::atomic<bool> producer_finished;

// Simple placeholder OCR pipeline class. Implementation can be added later.
class OCRPipeline {
public:
	OCRPipeline();

	// Placeholder recognition method: returns empty string for now.
	std::string recognize(const cv::Mat& img);
};
