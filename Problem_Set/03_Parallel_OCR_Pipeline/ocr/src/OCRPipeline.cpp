#include "OCRPipeline.hpp"

#include <string>

// Definitions for shared objects
std::queue<std::shared_ptr<ImageItem>> image_queue;
std::mutex queue_mutex;
std::counting_semaphore<1024> items_sem{0};
std::atomic<bool> producer_finished{false};

OCRPipeline::OCRPipeline() = default;

std::string OCRPipeline::recognize(const cv::Mat& /*img*/) {
    return std::string();
}
