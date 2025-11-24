#include "OCRServiceImpl.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

OCRServiceImpl::OCRServiceImpl(unsigned int num_workers) : num_workers_(num_workers == 0 ? std::max(1u, std::thread::hardware_concurrency()) : num_workers)
    , shutdown_(false)
{
    // Create OCR pipeline for each worker
    pipelines_.reserve(num_workers_);
    for (unsigned int i = 0; i < num_workers_; ++i) {
        pipelines_.push_back(std::make_unique<OCRPipeline>());
        if (!pipelines_.back()->isInitialized()) {
            std::cerr << "Failed to initialize OCR pipeline " << i << std::endl;
        }
    }
    
    std::cout << "OCR Service initialized with " << num_workers_ << " workers" << std::endl;
}

OCRServiceImpl::~OCRServiceImpl() {
    shutdown_.store(true);
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
                // Preprocess image (convert to grayscale for cleaning)
                cv::Mat processed_img;

                    if (img.channels() == 3)
                        cv::cvtColor(img, processed_img, cv::COLOR_BGR2GRAY);
                    else
                        processed_img = img.clone();
        
                    // Apply additional preprocessing
                    // Adaptive threshold for better OCR
                    cv::Mat cleaned;
                    cv::adaptiveThreshold(processed_img, cleaned, 255, 
                    cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2);
          
                    // Run OCR on the cleaned image
                    std::string ocr_text = pipelines_[idx]->recognize(cleaned);
               
                    result.set_success(true);
                    result.set_extracted_text(ocr_text);
          
                    // Encode cleaned image back to bytes (as PNG)
                    std::vector<uchar> cleaned_data;
                    cv::imencode(".png", cleaned, cleaned_data);
                    result.set_cleaned_image_data(cleaned_data.data(), cleaned_data.size());
        
                    std::cout << "Processed image: " << request.image_id() 
                    << " - Text length: " << ocr_text.length() << std::endl;
                }
            }
            catch (const std::exception& e) {
                result.set_success(false);
                result.set_error_message(std::string("Exception: ") + e.what());
                std::cerr << "Exception processing " << request.image_id() << ": " << e.what() << std::endl;
            }
     
            // Send result back (thread-safe)
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
