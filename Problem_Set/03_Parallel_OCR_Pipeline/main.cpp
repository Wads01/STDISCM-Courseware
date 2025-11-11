#include <iostream>
#include <thread>
#include <vector>
#include <filesystem>
#include <fstream>

#include "ocr/include/Producer.hpp"
#include "ocr/include/Consumer.hpp"
#include "ocr/include/OCRPipeline.hpp"

int main() {
	// DATASET_DIRECTORY is set by CMake to ${CMAKE_SOURCE_DIR}/dataset
	const std::filesystem::path default_dataset = DATASET_DIRECTORY;
	const std::filesystem::path base_dir = default_dataset.parent_path(); // should be ${CMAKE_SOURCE_DIR}

	std::cout << "Enter dataset directory name relative to project root (leave empty to use default): ";
	std::string input;
	std::getline(std::cin, input);

	std::filesystem::path dataset_dir;
	if (input.empty()) {
		dataset_dir = default_dataset;
	} else {
		dataset_dir = base_dir / input;
		// If invalid or empty directory, fallback to default
		if (!std::filesystem::exists(dataset_dir) || !std::filesystem::is_directory(dataset_dir)) {
			std::cerr << "Provided directory does not exist or is not a directory. Falling back to default dataset.\n";
			dataset_dir = default_dataset;
		}
	}

	// Ensure there is at least one image. If not, fallback to default.
	bool has_images = false;
	try {
		for (const auto& entry : std::filesystem::directory_iterator(dataset_dir)) {
			if (!entry.is_regular_file()) continue;
			auto ext = entry.path().extension().string();
			for (auto &c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") { has_images = true; break; }
		}
	} catch (...) {
		has_images = false;
	}
	if (!has_images && dataset_dir != default_dataset) {
		std::cerr << "No images found in provided directory. Falling back to default dataset.\n";
		dataset_dir = default_dataset;
	}

	std::cout << "Using dataset directory: " << dataset_dir.string() << "\n";

	// Prepare result CSV in project root
	std::filesystem::path result_csv = base_dir / "result.csv";
	{
		std::ofstream ofs(result_csv, std::ofstream::trunc);
		ofs << "ID,filename,extracted_text,processing_time_ms\n";
	}

	// Set shared CSV path and reset counter
	result_csv_path = result_csv.string();
	result_id_counter.store(0);

	// Number of consumer worker threads (at least2)
	unsigned int hw = std::thread::hardware_concurrency();
	unsigned int num_consumers = hw >1 ? hw :2u;
	if (num_consumers <2) num_consumers =2;

	std::cout << "Starting pipeline with " << num_consumers << " worker threads.\n";

	Producer producer(dataset_dir.string(), static_cast<int>(num_consumers));

	std::thread prod_thread(std::ref(producer));

	std::vector<std::thread> consumer_threads;
	consumer_threads.reserve(num_consumers);
	for (unsigned int i =0; i < num_consumers; ++i) {
		Consumer consumer(std::string(CLEANED_DIRECTORY));
		consumer_threads.emplace_back(std::move(consumer));
	}

	prod_thread.join();
	for (auto &t : consumer_threads) t.join();

	std::cout << "All done. Results written to: " << result_csv.string() << "\n";

	return 0;
}