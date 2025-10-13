#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

namespace sim {

    class Worker {
    public:
        using Task = std::function<void()>;

        explicit Worker(Task task, std::chrono::milliseconds sleepMs = std::chrono::milliseconds(200));
        ~Worker();

        Worker(const Worker&) = delete;
        Worker& operator=(const Worker&) = delete;

        void start();
        void stop();

        void setSleepDuration(std::chrono::milliseconds sleepMs);

        bool isRunning() const noexcept;

    private:
        void threadLoop();

        Task task_;
        std::chrono::milliseconds sleepMs_;
        std::atomic<bool> running_;
        std::thread workerThread_;
    };

}