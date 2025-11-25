#include "OCRServiceImpl.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>

OCRServiceImpl::OCRServiceImpl(unsigned int num_workers) : num_workers_(num_workers == 0 ? std::max(1u, std::thread::hardware_concurrency()) : num_workers)
    , shutdown_(false)
{
    // Create OCR pipeline for each worker
    pipelines_.reserve(num_workers_);
    for (unsigned int i = 0; i < num_workers_; ++i) {
        pipelines_.push_back(std::make_unique<OCRPipeline>());

        if (!pipelines_.back()->isInitialized())
            std::cerr << "Failed to initialize OCR pipeline " << i << std::endl;
    }
  
    std::cout << "OCR Service initialized with " << num_workers_ << " workers" << std::endl;
}

OCRServiceImpl::~OCRServiceImpl() {
    shutdown_.store(true);
}

static std::string cleanText(const std::string& text) {
    std::string result;
    result.reserve(text.length());
  
    // Alphanumeric characters only
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

grpc::Status OCRServiceImpl::ProcessImages(grpc::ServerContext* context, grpc::ServerReaderWriter<ocr::OCRResult, ocr::ImageRequest>* stream)
{
    std::cout << "Client connected for image processing" << std::endl;
    
    // Use thread pool approach: distribute work across pipelines
    std::atomic<int> pipeline_index{0};
    std::vector<std::thread> processing_threads;
    std::mutex stream_mutex;
    
    ocr::ImageRequest request;
    while (stream->Read(&request)) {
        std::cout << "Received image: " << request.image_id() << " (batch: " << request.batch_id() << ")" << std::endl;
        
        // Get next pipeline in round-robin fashion
        int idx = pipeline_index.fetch_add(1) % num_workers_;
    
        // Process in a separate thread to allow concurrent processing
        processing_threads.emplace_back([this, idx, request, stream, &stream_mutex]() {
            ocr::OCRResult result;
            result.set_image_id(request.image_id());
            result.set_batch_id(request.batch_id());
    
            try {
                // Decode image from bytes
                std::vector<uchar> image_data(request.image_data().begin(), request.image_data().end());
                cv::Mat img = cv::imdecode(image_data, cv::IMREAD_COLOR);
 
                if (img.empty()) {
                    result.set_success(false);
                    result.set_error_message("Failed to decode image");
                    std::cerr << "Failed to decode image: " << request.image_id() << std::endl;
                } else {
                    // Preprocess image using OCRPipeline method
                    cv::Mat cleaned = pipelines_[idx]->preprocessImage(img);
    
                    if (cleaned.empty()) {
                        result.set_success(false);
                        result.set_error_message("Failed to preprocess image");
                        std::cerr << "Failed to preprocess image: " << request.image_id() << std::endl;
                    } else {
                        std::string ocr_text = pipelines_[idx]->recognize(cleaned);
                        std::string cleaned_text = cleanText(ocr_text);
    
                        result.set_success(true);
                        result.set_extracted_text(cleaned_text);
        
                        std::vector<uchar> cleaned_data;
                        cv::imencode(".png", cleaned, cleaned_data);
                        result.set_cleaned_image_data(cleaned_data.data(), cleaned_data.size());

                        std::cout << "Processed image: " << request.image_id() << " - Text length: " << cleaned_text.length() << std::endl;
                    }
                }
            }
            catch (const std::exception& e) {
                result.set_success(false);
                result.set_error_message(std::string("Exception: ") + e.what());
                std::cerr << "Exception processing " << request.image_id() << ": " << e.what() << std::endl;
            }
     
            {
                std::lock_guard<std::mutex> lock(stream_mutex);
                if (!stream->Write(result))
                std::cerr << "Failed to write result for: " << request.image_id() << std::endl;
            }
        });
    }
    
    // Wait for all processing threads to complete
    for (auto& t : processing_threads) {
        if (t.joinable())
            t.join();
    }
    
    std::cout << "Client disconnected" << std::endl;
    return grpc::Status::OK;
}
