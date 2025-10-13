#include "Worker.hpp"

namespace sim {

    Worker::Worker(Task task, std::chrono::milliseconds sleepMs) : task_(std::move(task)), sleepMs_(sleepMs), running_(false) {}

    Worker::~Worker() {
        stop();
    }

    void Worker::start() {
        bool expected = false;
        if (!running_.compare_exchange_strong(expected, true))
            return;

        workerThread_ = std::thread(&Worker::threadLoop, this);
    }

    void Worker::stop() {
        bool expected = true;
        if (running_.compare_exchange_strong(expected, false)) {
            if (workerThread_.joinable())
                workerThread_.join();
        }
        else {
            if (workerThread_.joinable())
                workerThread_.join();
        }
    }

    void Worker::setSleepDuration(std::chrono::milliseconds sleepMs) {
        sleepMs_ = sleepMs;
    }

    bool Worker::isRunning() const noexcept {
        return running_.load(std::memory_order_relaxed);
    }

    void Worker::threadLoop() {
        while (running_.load(std::memory_order_relaxed)) {
            if (task_)
                task_();

            auto slept = std::chrono::milliseconds(0);
            const auto chunk = std::chrono::milliseconds(25);
        
            while (slept < sleepMs_ && running_.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::min(chunk, sleepMs_ - slept));
                slept += chunk;
            }
        }
    }

}