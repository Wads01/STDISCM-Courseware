#pragma once

#include <string>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include <semaphore>

#include <opencv2/opencv.hpp>

namespace tesseract { class TessBaseAPI; }

struct ImageItem {
	cv::Mat mat;
	std::string filename;
	bool sentinel = false;
};

extern std::queue<std::shared_ptr<ImageItem>> image_queue;
extern std::mutex queue_mutex;
extern std::counting_semaphore<1024> items_sem;
extern std::atomic<bool> producer_finished;

extern std::string result_csv_path;
extern std::mutex result_csv_mutex;
extern std::atomic<int> result_id_counter;

class OCRPipeline {
public:
	OCRPipeline();
	~OCRPipeline();

	std::string recognize(const cv::Mat& img);
	
	bool isInitialized() const;

private:
	std::unique_ptr<tesseract::TessBaseAPI> api_;
};
