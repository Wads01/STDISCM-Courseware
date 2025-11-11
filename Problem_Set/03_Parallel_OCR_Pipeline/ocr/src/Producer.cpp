#include "Producer.hpp"
#include "../include/OCRPipeline.hpp"

#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <memory>
#include <string>
#include <mutex>
#include <semaphore>
#include <atomic>
#include <vector>
#include <cctype>

Producer::Producer(std::string dataset_dir, int consumer_count)
	: dataset_dir_(std::move(dataset_dir)), consumer_count_(consumer_count) {}

static std::vector<std::filesystem::path> collect_png_paths(const std::string& dataset_dir) {
	std::vector<std::filesystem::path> paths;
	try {
		if (!std::filesystem::exists(dataset_dir) || !std::filesystem::is_directory(dataset_dir)) {
			return paths;
		}
		for (const auto& entry : std::filesystem::directory_iterator(dataset_dir)) {
			if (!entry.is_regular_file()) continue;
			auto path = entry.path();
			auto ext = path.extension().string();
			std::string ext_l;
			ext_l.reserve(ext.size());
			for (char c : ext) ext_l.push_back(std::tolower(static_cast<unsigned char>(c)));
			if (ext_l != ".png" && ext_l != ".jpg" && ext_l != ".jpeg") continue;
			paths.push_back(path);
		}
		std::sort(paths.begin(), paths.end());
	}
	catch (const std::exception& ex) {
		std::cerr << "collect_png_paths exception: " << ex.what() << "\n";
	}
	return paths;
}

void Producer::operator()() {
	try {
		if (!std::filesystem::exists(dataset_dir_) || !std::filesystem::is_directory(dataset_dir_)) {
			std::cerr << "Dataset directory '" << dataset_dir_ << "' does not exist or is not a directory.\n";
			producer_finished.store(true);
			for (int i =0; i < consumer_count_; ++i) {
				auto end_item = std::make_shared<ImageItem>();
				end_item->sentinel = true;
				{
					std::lock_guard<std::mutex> lock(queue_mutex);
					image_queue.push(end_item);
				}
				items_sem.release();
			}
			return;
		}

		auto paths = collect_png_paths(dataset_dir_);
		for (const auto& path : paths) {
			auto fullpath = path.string();
			cv::Mat img = cv::imread(fullpath, cv::IMREAD_COLOR);
			if (img.empty()) {
				std::cerr << "Warning: failed to load image: " << fullpath << "\n";
				continue;
			}

			auto item = std::make_shared<ImageItem>();
			item->mat = std::move(img);
			item->filename = path.filename().string();
			{
				std::lock_guard<std::mutex> lock(queue_mutex);
				image_queue.push(item);
			}
			items_sem.release();
			std::cout << "[Producer] Enqueued: " << item->filename << "\n";
		}
	}
	catch (const std::exception& ex) {
		std::cerr << "Producer exception: " << ex.what() << "\n";
	}
	// Push sentinel for each consumer
	for (int i =0; i < consumer_count_; ++i) {
		auto end_item = std::make_shared<ImageItem>();
		end_item->sentinel = true;
		std::lock_guard<std::mutex> lock(queue_mutex);
		image_queue.push(end_item);
		items_sem.release();
	}

	producer_finished.store(true);
	std::cout << "[Producer] Finished scanning dataset.\n";
}
