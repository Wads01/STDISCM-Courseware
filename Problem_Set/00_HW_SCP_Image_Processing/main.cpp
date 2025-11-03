#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <string>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <semaphore>
#include <cctype>

#include <opencv2/opencv.hpp>

struct ImageItem {
	cv::Mat mat;
	std::string filename;
	bool sentinel = false;
};

static std::queue<std::shared_ptr<ImageItem>> image_queue;
static std::mutex queue_mutex;
static std::counting_semaphore<1024> items_sem(0);
static std::atomic<bool> producer_finished{ false };

std::vector<std::filesystem::path> collect_png_paths(const std::string& dataset_dir);
bool save_cleaned_image(const std::string& output_dir, const std::string& filename, const cv::Mat& img);

void producer(const std::string& dataset_dir);
void consumer(const std::string& output_dir);
void process_image_for_cleaning(const cv::Mat& src, cv::Mat& dst);

int main() {
	const std::string dataset_dir = DATASET_DIRECTORY;
	const std::string output_dir = CLEANED_DIRECTORY;

	std::thread prod_thread(producer, dataset_dir);
	std::thread cons_thread(consumer, output_dir);

	prod_thread.join();
	cons_thread.join();

	std::cout << "All done.\n";

	return 0;
}

std::vector<std::filesystem::path> collect_png_paths(const std::string& dataset_dir) {
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
			if (ext_l != ".png") continue;
			paths.push_back(path);
		}
		std::sort(paths.begin(), paths.end());
	} catch (const std::exception& ex) {
		std::cerr << "collect_png_paths exception: " << ex.what() << "\n";
	}
	return paths;
}

bool save_cleaned_image(const std::string& output_dir, const std::string& filename, const cv::Mat& img) {
	try {
		std::filesystem::create_directories(output_dir);
		std::string outpath = (std::filesystem::path(output_dir) / (std::string("cleaned_") + filename)).string();
		return cv::imwrite(outpath, img);
	} catch (const std::exception& ex) {
		std::cerr << "save_cleaned_image exception for " << filename << ": " << ex.what() << "\n";
		return false;
	}
}

void producer(const std::string& dataset_dir) {
	try {
		if (!std::filesystem::exists(dataset_dir) || !std::filesystem::is_directory(dataset_dir)) {
			std::cerr << "Dataset directory '" << dataset_dir << "' does not exist or is not a directory.\n";
			producer_finished.store(true);
			auto end_item = std::make_shared<ImageItem>();
			end_item->sentinel = true;
			{
				std::lock_guard<std::mutex> lock(queue_mutex);
				image_queue.push(end_item);
			}
			items_sem.release();
			return;
		}

		auto paths = collect_png_paths(dataset_dir);
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
	} catch (const std::exception& ex) {
		std::cerr << "Producer exception: " << ex.what() << "\n";
	}
	{
		auto end_item = std::make_shared<ImageItem>();
		end_item->sentinel = true;
		std::lock_guard<std::mutex> lock(queue_mutex);
		image_queue.push(end_item);
	}

	items_sem.release();
	producer_finished.store(true);
	std::cout << "[Producer] Finished scanning dataset.\n";
}

void consumer(const std::string& output_dir) {
	size_t processed = 0;

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
		} catch (const std::exception& ex) {
			std::cerr << "[Consumer] Processing error for " << item->filename << ": " << ex.what() << "\n";
			continue;
		}

		if (!save_cleaned_image(output_dir, item->filename, cleaned)) {
			std::cerr << "[Consumer] Failed to write cleaned image for: " << item->filename << "\n";
		} else {
			std::cout << "[Consumer] Processed and saved: " << item->filename << "\n";
			++processed;
		}
	}

	std::cout << "Total processed: " << processed << "\n";
}

void process_image_for_cleaning(const cv::Mat& src, cv::Mat& dst) {
	cv::Mat gray;
	if (src.channels() == 3) cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	else gray = src.clone();

	cv::medianBlur(gray, gray, 3);

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
	cv::morphologyEx(gray, gray, cv::MORPH_OPEN, kernel);

	cv::adaptiveThreshold(gray, dst, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 15, 10);
}