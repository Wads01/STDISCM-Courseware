# pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "Algorithm.hpp"

class Worker {
public:
	Worker(int id);
	~Worker();
	void assignTask(std::function<void()> task);

private:
	void run();
	int worker_id;
	std::thread worker_thread;
	std::mutex mtx;
	std::condition_variable cv;
	bool stop;

	std::vector<std::function<void()>> tasks;
};
