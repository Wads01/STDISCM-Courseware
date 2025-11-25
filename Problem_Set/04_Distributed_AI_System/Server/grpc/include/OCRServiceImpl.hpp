#pragma once

#include <grpcpp/grpcpp.h>
#include "ocr_service.grpc.pb.h"
#include "OCRPipeline.hpp"
#include <memory>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <atomic>
#include <chrono>

struct OCRWorkItem {
	ocr::ImageRequest request;
	grpc::ServerReaderWriter<ocr::OCRResult, ocr::ImageRequest>* stream;
	std::shared_ptr<std::mutex> stream_mutex;
	std::shared_ptr<std::atomic<size_t>> pending_work_count;
	bool sentinel = false;
};

class OCRServiceImpl final : public ocr::OCRService::Service {
public:
	OCRServiceImpl(unsigned int num_workers = 0, size_t max_queue_size = 100, unsigned int processing_delay_ms = 0);
	~OCRServiceImpl();

	grpc::Status ProcessImages(grpc::ServerContext* context, grpc::ServerReaderWriter<ocr::OCRResult, ocr::ImageRequest>* stream) override;

private:
	void workerThread(unsigned int worker_id);
	void processWorkItem(const OCRWorkItem& work_item, unsigned int worker_id);
	
	std::vector<std::unique_ptr<OCRPipeline>> pipelines_;
	std::vector<std::thread> workers_;
	unsigned int num_workers_;
	
	// Bounded queue with flow control
	std::queue<OCRWorkItem> work_queue_;
	std::mutex queue_mutex_;
	std::condition_variable queue_cv_;
	std::counting_semaphore<1024> queue_slots_;
	std::atomic<bool> shutdown_;
	size_t max_queue_size_;
	
	std::chrono::milliseconds processing_delay_;
};
