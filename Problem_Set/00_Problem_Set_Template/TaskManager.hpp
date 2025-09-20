#pragma once

#include <vector>
#include <memory>
#include <unordered_set>
#include <random>

#include "Worker.hpp"
#include "Algorithm.hpp"

class TaskManager {
public:
    static TaskManager& getInstance();

    void addWorker();
    void assignTaskToWorker(int workerIndex, const Algorithm& task);

    TaskManager(const TaskManager&) = delete;
    TaskManager& operator=(const TaskManager&) = delete;
    TaskManager(TaskManager&&) = delete;
    TaskManager& operator=(TaskManager&&) = delete;

private:
    TaskManager() = default;

    std::vector<std::unique_ptr<Worker>> workers;

    static int generateRandomWorkerID();
    static std::unordered_set<int> used_ids;
    static std::mt19937 rng;
    static std::uniform_int_distribution<int> dist;
};