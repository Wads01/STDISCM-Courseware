#include <iostream>
#include <vector>
#include <thread>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <algorithm>

#include "OCRPipeline.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string input_dir;
    if (argc >1) {
        input_dir = argv[1];
    } else {
        std::cout << "Usage: " << argv[0] << " <image-directory>\n";
        std::cout << "Please provide a directory containing images to process." << std::endl;
        return 1;
    }

    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        std::cerr << "Provided path is not a directory: " << input_dir << std::endl;
        return 1;
    }

    // Prepare result CSV - assign to the global declared in OCRPipeline.hpp
    result_csv_path = (fs::path(input_dir) / "ocr_results.csv").string();
    {
        std::ofstream hdr(result_csv_path, std::ios::out | std::ios::trunc);
        hdr << "id,filename,result" << std::endl;
    }

    // Initialize OCR
    OCRPipeline pipeline;
    if (!pipeline.isInitialized()) {
        std::cerr << "OCR pipeline failed to initialize. Exiting." << std::endl;
        return 1;
    }

    // Gather image files (jpg/jpeg/png/bmp/tiff)
    std::vector<fs::path> images;
    for (auto& p : fs::directory_iterator(input_dir)) {
        if (!p.is_regular_file()) continue;
        auto ext = p.path().extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
        if (ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".bmp" || ext == L".tiff") {
            images.push_back(p.path());
        }
    }

    if (images.empty()) {
        std::cout << "No images found in directory: " << input_dir << std::endl;
        return 0;
    }

    std::cout << "Found " << images.size() << " images. Starting processing..." << std::endl;

    // Start worker threads
    unsigned int num_workers = std::max(1u, std::thread::hardware_concurrency() >0 ? std::thread::hardware_concurrency() :4u);
    std::vector<std::thread> workers;
    workers.reserve(num_workers);

    for (unsigned int i =0; i < num_workers; ++i) {
        workers.emplace_back([&pipeline]() {
            while (true) {
                // Wait for an item
                items_sem.acquire();

                std::shared_ptr<ImageItem> item;
                {
                    std::lock_guard<std::mutex> lk(queue_mutex);
                    if (image_queue.empty()) {
                        // Spurious wake or producer finished; check termination
                        if (producer_finished.load()) {
                            // Release for other workers and exit
                            items_sem.release();
                            break;
                        }
                        continue;
                    }
                    item = image_queue.front();
                    image_queue.pop();
                }

                if (!item) continue;
                if (item->sentinel) {
                    // Push sentinel back for other workers and exit
                    std::lock_guard<std::mutex> lk(queue_mutex);
                    image_queue.push(item);
                    items_sem.release();
                    break;
                }

                // Run OCR
                auto start = std::chrono::high_resolution_clock::now();
                std::string ocr_text = pipeline.recognize(item->mat);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> elapsed = end - start;

                // Write result to CSV (thread-safe)
                int id = result_id_counter.fetch_add(1);
                {
                    std::lock_guard<std::mutex> lk(result_csv_mutex);
                    std::ofstream out(result_csv_path, std::ios::out | std::ios::app);
                    if (out) {
                        // Simple CSV escaping: wrap result in quotes and replace quotes inside
                        std::string safe = ocr_text;
                        size_t pos =0;
                        while ((pos = safe.find('"', pos)) != std::string::npos) {
                            safe.insert(pos,1, '"');
                            pos +=2;
                        }
                        out << id << "," << '"' << item->filename << '"' << "," << '"' << safe << '"' << "\n";
                    }
                }

                std::cout << "Processed: " << item->filename << " (id=" << id << ", time=" << elapsed.count() << "ms)" << std::endl;
            }
        });
    }

    // Producer: enqueue images
    for (const auto& p : images) {
        cv::Mat img = cv::imread(p.string(), cv::IMREAD_COLOR);
        if (img.empty()) {
            std::cerr << "Failed to read image: " << p << std::endl;
            continue;
        }
        auto item = std::make_shared<ImageItem>();
        item->mat = img;
        item->filename = p.filename().string();
        item->sentinel = false;

        {
            std::lock_guard<std::mutex> lk(queue_mutex);
            image_queue.push(item);
        }
        items_sem.release();
    }

    // Signal producer finished
    producer_finished.store(true);

    // Wait for workers to finish processing
    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }

    std::cout << "All images processed. Results in: " << result_csv_path << std::endl;
    return 0;
}
