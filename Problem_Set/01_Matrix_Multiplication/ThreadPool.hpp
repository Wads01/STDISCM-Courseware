#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>

class ThreadPool {
public:
	ThreadPool(size_t numThreads) : stop(false) {
		for (size_t i = 0; i < numThreads; ++i) {
			workers.emplace_back([this] {
				for (;;) {
					std::unique_lock<std::mutex> lock(this->qMutex);
					cv.wait(lock, [this] { return this->stop || !this->tasks.empty(); });

					if (this->stop && this->tasks.empty()) return;

					auto task = std::move(this->tasks.front());
					tasks.pop();
					lock.unlock();
					task();
				}
			});
		}
	}

	~ThreadPool() {
		std::unique_lock<std::mutex> lock(qMutex);
		stop = true;
		lock.unlock();
		cv.notify_all();

		for (std::thread& worker : workers) {
			if (worker.joinable()) worker.join();
		}
	}

	template<class F>
	void enqueue(F&& task) {
		{
			std::unique_lock<std::mutex> lock(qMutex);
			tasks.emplace(std::forward<F>(task));
			lock.unlock();
		}
		cv.notify_one();
	}

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	std::mutex qMutex;
	std::condition_variable cv;
	bool stop;
};