#include <iostream>
#include <thread>

#include "ocr/include/Producer.hpp"
#include "ocr/include/Consumer.hpp"
#include "ocr/include/OCRPipeline.hpp"

int main() {
	const std::string dataset_dir = DATASET_DIRECTORY;
	const std::string output_dir = CLEANED_DIRECTORY;

	Producer producer(dataset_dir);
	Consumer consumer(output_dir);

	std::thread prod_thread(std::ref(producer));
	std::thread cons_thread(std::ref(consumer));

	prod_thread.join();
	cons_thread.join();

	std::cout << "All done.\n";

	return 0;
}