#include "Worker.hpp"
#include <iostream>
#include <thread>
#include <chrono>

Worker::Worker(int id) : worker_id(id), stop(false) {
    worker_thread = std::thread(&Worker::run, this);
}

Worker::~Worker() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        stop = true;
        cv.notify_all();
    }
    if (worker_thread.joinable()) {
        worker_thread.join();
    }
}

void Worker::assignTask(Algorithm task) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.emplace_back(std::move(task));
        cv.notify_all();
    }
}

void Worker::run() {
    while (true) {
        Algorithm task;
        bool hasTask = false;

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return !tasks.empty() || stop; });

            if (stop && tasks.empty()) break;

            if (!tasks.empty()) {
                task = std::move(tasks.back());
                tasks.pop_back();
                hasTask = true;
            }
        }

        if (hasTask) {
            task.execute();
        }
    }
}
