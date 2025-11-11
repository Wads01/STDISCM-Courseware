#include "Consumer.hpp"
#include "../include/OCRPipeline.hpp"

#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <memory>
#include <mutex>
#include <fstream>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <cctype>

static std::string escapeCsvText(const std::string& text) {
	std::string result;
	
	for (char c : text) {
		if (std::isalnum(static_cast<unsigned char>(c))) {
			result += c;
		} else if (std::isspace(static_cast<unsigned char>(c))) {
			result += ' ';
		}
	}
	
	result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
	result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), result.end());
	
	std::string compressed;
	bool lastWasSpace = false;
	for (char c : result) {
		if (std::isspace(static_cast<unsigned char>(c))) {
			if (!lastWasSpace) {
				compressed += ' ';
				lastWasSpace = true;
			}
		} else {
			compressed += c;
			lastWasSpace = false;
		}
	}
	
	return compressed;
}

static bool save_cleaned_image(const std::string& output_dir, const std::string& filename, const cv::Mat& img) {
	try {
		std::filesystem::create_directories(output_dir);
		std::string outpath = (std::filesystem::path(output_dir) / (std::string("cleaned_") + filename)).string();
		return cv::imwrite(outpath, img);
	}
	catch (const std::exception& ex) {
		std::cerr << "save_cleaned_image exception for " << filename << ": " << ex.what() << "\n";
		return false;
	}
}

static void process_image_for_cleaning(const cv::Mat& src, cv::Mat& dst) {
	cv::Mat gray;
	if (src.channels() ==3) cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	else gray = src.clone();

	cv::medianBlur(gray, gray,3);

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3,3));
	cv::morphologyEx(gray, gray, cv::MORPH_OPEN, kernel);

	cv::adaptiveThreshold(gray, dst,255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY,15,10);
}

Consumer::Consumer(std::string output_dir) : output_dir_(std::move(output_dir)), pipeline_(std::make_unique<OCRPipeline>()) {}

void Consumer::operator()() {
	size_t processed = 0;

	if (!pipeline_ || !pipeline_->isInitialized()) {
		std::cerr << "[Consumer] OCR Pipeline not initialized, exiting consumer thread.\n";
		return;
	}

	while (true) {
		items_sem.acquire();

		std::shared_ptr<ImageItem> item;
		{
			std::lock_guard<std::mutex> lock(queue_mutex);
			if (image_queue.empty()) continue;
			item = image_queue.front();
			image_queue.pop();
		}

		if (!item) continue;
		if (item->sentinel) {
			std::cout << "[Consumer] Received sentinel, exiting.\n";
			break;
		}

		cv::Mat cleaned;
		std::string extracted;
		auto start = std::chrono::steady_clock::now();
		try {
			process_image_for_cleaning(item->mat, cleaned);
			extracted = pipeline_->recognize(cleaned);
		}
		catch (const std::exception& ex) {
			std::cerr << "[Consumer] Processing error for " << item->filename << ": " << ex.what() << "\n";
			continue;
		}
		auto end = std::chrono::steady_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		if (!save_cleaned_image(output_dir_, item->filename, cleaned)) {
			std::cerr << "[Consumer] Failed to write cleaned image for: " << item->filename << "\n";
		} else {
			++processed;
		}

		int id = ++result_id_counter;
		{
			std::lock_guard<std::mutex> lock(result_csv_mutex);
			
			std::string csv_path = result_csv_path;
			std::replace(csv_path.begin(), csv_path.end(), '/', '\\');
			
			std::ofstream ofs(csv_path, std::ios::app);
			if (ofs) {
 				std::string escaped_text = escapeCsvText(extracted);
				ofs << id << ",\"" << item->filename << "\",\"" << escaped_text << "\"," << ms << "\n";
			}
		}
	}

	std::cout << "Total processed: " << processed << "\n";
}
