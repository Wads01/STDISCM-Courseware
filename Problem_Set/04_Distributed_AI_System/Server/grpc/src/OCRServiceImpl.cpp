#include "OCRServiceImpl.hpp"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <thread>

OCRServiceImpl::OCRServiceImpl(unsigned int num_workers, size_t max_queue_size)
    : num_workers_(num_workers == 0 ? std::max(1u, std::thread::hardware_concurrency()) : num_workers)
    , shutdown_(false)
    , max_queue_size_(max_queue_size)
    , queue_slots_(max_queue_size)
{
    // Create OCR pipeline for each worker
    pipelines_.reserve(num_workers_);
    for (unsigned int i = 0; i < num_workers_; ++i) {
        pipelines_.push_back(std::make_unique<OCRPipeline>());
        if (!pipelines_.back()->isInitialized())
            std::cerr << "Failed to initialize OCR pipeline " << i << std::endl;
    }

    workers_.reserve(num_workers_);
    for (unsigned int i = 0; i < num_workers_; ++i)
        workers_.emplace_back(&OCRServiceImpl::workerThread, this, i);

  std::cout << "OCR Service initialized with " << num_workers_ << " workers and max queue size "
        << max_queue_size_ << std::endl;
}

OCRServiceImpl::~OCRServiceImpl() {
    shutdown_.store(true);

    // Send sentinel work items to wake up all workers
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        for (unsigned int i = 0; i < num_workers_; ++i) {
            OCRWorkItem sentinel;
            sentinel.sentinel = true;
            work_queue_.push(sentinel);
        }
    }
    queue_cv_.notify_all();

    // Wait for all workers to finish
    for (auto& worker : workers_) {
        if (worker.joinable())
            worker.join();
    }

    std::cout << "OCR Service shutdown complete" << std::endl;
}

// Helper function to clean OCR text output
static std::string cleanText(const std::string& text) {
  std::string result;
  result.reserve(text.length());

    // Only keep alphanumeric characters and spaces
    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c)) || std::isspace(static_cast<unsigned char>(c)))
            result += c;
    }

    // Remove trailing whitespaces and newlines
    size_t end = result.find_last_not_of(" \t\n\r\f\v");
    if (end != std::string::npos)
        result.erase(end + 1);
    else
        result.clear();

    // Remove leading whitespaces
    size_t start = result.find_first_not_of(" \t\n\r\f\v");
    if (start != std::string::npos)
        result.erase(0, start);

    return result;
}

void OCRServiceImpl::workerThread(unsigned int worker_id) {
    std::cout << "Worker " << worker_id << " started" << std::endl;

    while (!shutdown_.load()) {
        OCRWorkItem work_item;

        // Wait for work
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return shutdown_.load() || !work_queue_.empty(); });

            if (work_queue_.empty())
                continue;

            work_item = work_queue_.front();
            work_queue_.pop();
        }

        // Check for sentinel
        if (work_item.sentinel)
            break;

        // Process the work item
        processWorkItem(work_item, worker_id);

        // Release a queue slot
        queue_slots_.release();
    }

    std::cout << "Worker " << worker_id << " stopped" << std::endl;
}

void OCRServiceImpl::processWorkItem(const OCRWorkItem& work_item, unsigned int worker_id) {
    const auto& request = work_item.request;

    ocr::OCRResult result;
    result.set_image_id(request.image_id());
    result.set_batch_id(request.batch_id());

    // Report current queue depth to client for monitoring
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        result.set_queue_depth(static_cast<int>(work_queue_.size()));
    }

    try {
        // Decode image from bytes
        std::vector<uchar> image_data(request.image_data().begin(), request.image_data().end());
        cv::Mat img = cv::imdecode(image_data, cv::IMREAD_COLOR);

        if (img.empty()) {
            result.set_success(false);
            result.set_error_message("Failed to decode image");
            std::cerr << "Worker " << worker_id << ": Failed to decode image: " << request.image_id() << std::endl;
        } else {
            // Preprocess image using OCRPipeline method
            cv::Mat cleaned = pipelines_[worker_id]->preprocessImage(img);

            if (cleaned.empty()) {
                result.set_success(false);
                result.set_error_message("Failed to preprocess image");
                std::cerr << "Worker " << worker_id << ": Failed to preprocess image: " << request.image_id() << std::endl;
            } else {
                // Run OCR on the cleaned image
                std::string ocr_text = pipelines_[worker_id]->recognize(cleaned);

                // Clean the OCR text output
                std::string cleaned_text = cleanText(ocr_text);

                result.set_success(true);
                result.set_extracted_text(cleaned_text);

                // Encode cleaned image back to bytes (as PNG)
                std::vector<uchar> cleaned_data;
                cv::imencode(".png", cleaned, cleaned_data);
                result.set_cleaned_image_data(cleaned_data.data(), cleaned_data.size());

                std::cout << "Worker " << worker_id << ": Processed image: " << request.image_id()
                        << " - Text length: " << cleaned_text.length() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        result.set_success(false);
        result.set_error_message(std::string("Exception: ") + e.what());
        std::cerr << "Worker " << worker_id << ": Exception processing " << request.image_id() << ": " << e.what() << std::endl;
    }

    // Send result back (thread-safe)
    {
        std::lock_guard<std::mutex> lock(*work_item.stream_mutex);
        if (!work_item.stream->Write(result))
            std::cerr << "Worker " << worker_id << ": Failed to write result for: " << request.image_id() << std::endl;
    }
}

grpc::Status OCRServiceImpl::ProcessImages(grpc::ServerContext* context, grpc::ServerReaderWriter<ocr::OCRResult, ocr::ImageRequest>* stream) {
    std::cout << "Client connected for image processing" << std::endl;

    std::mutex stream_mutex;
    size_t images_received = 0;
    std::atomic<size_t> current_queue_size{0};

    ocr::ImageRequest request;
    while (stream->Read(&request)) {
        images_received++;
        std::cout << "Received image " << images_received << ": " << request.image_id() << " (batch: " << request.batch_id() << ")" << std::endl;

        // Wait for an available queue slot (backpressure mechanism)
        // This blocks if the queue is full, preventing server overload
        queue_slots_.acquire();

        if (shutdown_.load()) {
            std::cout << "Server shutting down, rejecting new work" << std::endl;
            queue_slots_.release();
            break;
        }

        // Add work item to queue
        OCRWorkItem work_item;
        work_item.request = request;
        work_item.stream = stream;
        work_item.stream_mutex = &stream_mutex;
        work_item.sentinel = false;

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            work_queue_.push(work_item);
            current_queue_size.store(work_queue_.size());
        }

        queue_cv_.notify_one();

        std::cout << "Queued image for processing (queue size: " << current_queue_size.load() << "/" << max_queue_size_ << ")" << std::endl;
    }

    std::cout << "Client disconnected (received " << images_received << " images)" << std::endl;
    return grpc::Status::OK;
}
