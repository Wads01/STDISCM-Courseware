#include "Consumer.hpp"
#include "../include/OCRPipeline.hpp"

#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <memory>
#include <mutex>

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

Consumer::Consumer(std::string output_dir) : output_dir_(std::move(output_dir)) {}

void Consumer::operator()() {
	size_t processed =0;

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
		try {
			process_image_for_cleaning(item->mat, cleaned);
		}
		catch (const std::exception& ex) {
			std::cerr << "[Consumer] Processing error for " << item->filename << ": " << ex.what() << "\n";
			continue;
		}

		if (!save_cleaned_image(output_dir_, item->filename, cleaned)) {
			std::cerr << "[Consumer] Failed to write cleaned image for: " << item->filename << "\n";
		}
		else {
			std::cout << "[Consumer] Processed and saved: " << item->filename << "\n";
			++processed;
		}
	}

	std::cout << "Total processed: " << processed << "\n";
}
