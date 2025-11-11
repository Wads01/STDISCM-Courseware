#include "OCRPipeline.hpp"

#include <string>
#include <sstream>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

// Definitions for shared objects
std::queue<std::shared_ptr<ImageItem>> image_queue;
std::mutex queue_mutex;
std::counting_semaphore<1024> items_sem{0};
std::atomic<bool> producer_finished{false};

// CSV/result globals
std::string result_csv_path;
std::mutex result_csv_mutex;
std::atomic<int> result_id_counter{0};

OCRPipeline::OCRPipeline()
{
	api_ = std::make_unique<tesseract::TessBaseAPI>();
	
	// Try to find tessdata directory and set TESSDATA_PREFIX if needed
	std::string tessdata_path;
	
	// First check if TESSDATA_PREFIX is already set
	const char* env_tessdata = std::getenv("TESSDATA_PREFIX");
	if (env_tessdata && std::filesystem::exists(env_tessdata)) {
		tessdata_path = env_tessdata;
	}
	else {
		// Try to find tessdata relative to the current executable
		std::filesystem::path current_path = std::filesystem::current_path();
		std::vector<std::string> possible_paths = {
			(current_path / "tessdata").string(),
			(current_path / ".." / "tessdata").string(),
			"C:\\Program Files\\Tesseract-OCR\\tessdata",
			"C:\\vcpkg\\installed\\x64-windows\\tools\\tesseract\\tessdata",
			"D:\\Misc\\vcpkg\\installed\\x64-windows\\tools\\tesseract\\tessdata"
		};
		
		for (const auto& path : possible_paths) {
			if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
				// Check if eng.traineddata or similar exists
				if (std::filesystem::exists(std::filesystem::path(path) / "eng.traineddata") ||
					std::filesystem::exists(std::filesystem::path(path) / "eng_fast.traineddata")) {
					tessdata_path = path;
					break;
				}
			}
		}
	}
	
	// Set TESSDATA_PREFIX if we found a valid path
	if (!tessdata_path.empty()) {
		#ifdef _WIN32
		_putenv_s("TESSDATA_PREFIX", tessdata_path.c_str());
		#else
		setenv("TESSDATA_PREFIX", tessdata_path.c_str(), 1);
		#endif
		std::cout << "Set TESSDATA_PREFIX to: " << tessdata_path << std::endl;
	}
	
	// Initialize with eng_fast since we have that file
	int init_result = api_->Init(tessdata_path.empty() ? nullptr : tessdata_path.c_str(), "eng_fast");
	if (init_result != 0) {
		std::cerr << "Could not initialize tesseract with eng_fast language model.\n";
		std::cerr << "Tessdata path: " << (tessdata_path.empty() ? "default" : tessdata_path) << std::endl;
		api_.reset();
	} else {
		std::cout << "Successfully initialized Tesseract with eng_fast language model." << std::endl;
		// Set OCR Engine Mode to better handle the image processing
		api_->SetPageSegMode(tesseract::PSM_AUTO);
	}
}

OCRPipeline::~OCRPipeline() {
	if (api_) api_->End();
}

bool OCRPipeline::isInitialized() const {
	return api_ != nullptr;
}

// recognition method
std::string OCRPipeline::recognize(const cv::Mat& img) {
	if (!api_) {
		std::cerr << "Tesseract API not initialized!" << std::endl;
		return std::string();
	}

	if (img.empty()) {
		std::cerr << "Empty image provided to OCR!" << std::endl;
		return std::string();
	}

	try {
		// Convert to grayscale if needed and ensure proper format
		cv::Mat processed_img;
		if (img.channels() == 3) {
			cv::cvtColor(img, processed_img, cv::COLOR_BGR2GRAY);
		} else if (img.channels() == 1) {
			processed_img = img.clone();
		} else {
			std::cerr << "Unsupported number of channels: " << img.channels() << std::endl;
			return std::string();
		}

		// Ensure 8-bit depth
		if (processed_img.depth() != CV_8U) {
			processed_img.convertTo(processed_img, CV_8U);
		}

		// Set image data directly to Tesseract (safer than using Leptonica manually)
		api_->SetImage(processed_img.data, processed_img.cols, processed_img.rows, 
					   processed_img.channels(), processed_img.step[0]);

		// Get the text
		char* out = api_->GetUTF8Text();
		std::string result;
		if (out) {
			result = std::string(out);
			delete[] out;
		}

		return result;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception in OCR recognize: " << e.what() << std::endl;
		return std::string();
	}
	catch (...) {
		std::cerr << "Unknown exception in OCR recognize" << std::endl;
		return std::string();
	}
}
