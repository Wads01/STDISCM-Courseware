#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <future>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <type_traits>
#include <utility>

// Simple ThreadPool implementation
// - Construct with number of worker threads (default: hardware concurrency or 2)
// - enqueue() accepts any callable and returns a std::future for the result
// - Destructor joins all threads after completing pending tasks

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency()) {
        if (numThreads == 0) numThreads = 2; // fallback
        stop.store(false);
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return stop.load() || !tasks.empty(); });

                        if (stop.load() && tasks.empty())
                            return; // exit thread

                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    // Execute the task outside the lock
                    task();
                }
            });
        }
    }

    // Non-copyable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Enqueue a task. Returns a future holding the result.
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
    {
        using return_type = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

        auto taskPtr = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = taskPtr->get_future();
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (stop.load())
                throw std::runtime_error("enqueue on stopped ThreadPool");

            // push a void()-callable that runs the packaged_task
            tasks.emplace([taskPtr]() { (*taskPtr)(); });
        }
        condition.notify_one();
        return res;
    }

    // Destructor: stop and join all threads
    ~ThreadPool() {
        stop.store(true);
        condition.notify_all();
        for (std::thread &worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop{false};
};