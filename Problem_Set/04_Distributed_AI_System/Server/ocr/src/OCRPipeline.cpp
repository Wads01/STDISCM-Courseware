#include "OCRPipeline.hpp"

#include <string>
#include <sstream>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

std::queue<std::shared_ptr<ImageItem>> image_queue;
std::mutex queue_mutex;
std::counting_semaphore<1024> items_sem{0};
std::atomic<bool> producer_finished{false};

std::string result_csv_path;
std::mutex result_csv_mutex;
std::atomic<int> result_id_counter{0};

OCRPipeline::OCRPipeline()
{
	api_ = std::make_unique<tesseract::TessBaseAPI>();
	
	const std::string tessdata_path_str = TESSDATA_DIRECTORY;
	
	#ifdef _WIN32
		_putenv_s("TESSDATA_PREFIX", tessdata_path_str.c_str());
	#else
		setenv("TESSDATA_PREFIX", tessdata_path_str.c_str(), 1);
	#endif
	
	if (api_->Init(tessdata_path_str.c_str(), "eng_fast") != 0) {
		std::cerr << "Could not initialize tesseract with eng_fast language model.\n";
		std::cerr << "Tessdata path: " << tessdata_path_str << std::endl;
		api_.reset();
	} else {
		api_->SetPageSegMode(tesseract::PSM_AUTO);
	}
}

OCRPipeline::~OCRPipeline() {
	if (api_) api_->End();
}

bool OCRPipeline::isInitialized() const {
	return api_ != nullptr;
}

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
		cv::Mat processed_img;
		if (img.channels() == 3) {
			cv::cvtColor(img, processed_img, cv::COLOR_BGR2GRAY);
		} else if (img.channels() == 1) {
			processed_img = img.clone();
		} else {
			std::cerr << "Unsupported number of channels: " << img.channels() << std::endl;
			return std::string();
		}

		if (processed_img.depth() != CV_8U) {
			processed_img.convertTo(processed_img, CV_8U);
		}

		api_->SetImage(processed_img.data, processed_img.cols, processed_img.rows, 
					   processed_img.channels(), processed_img.step[0]);

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
