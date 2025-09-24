#include "TaskManager.hpp"

std::unordered_set<int> TaskManager::used_ids;
std::mt19937 TaskManager::rng(std::random_device{}());
std::uniform_int_distribution<int> TaskManager::dist(0, 9999);

TaskManager& TaskManager::getInstance() {
    static TaskManager instance;
    return instance;
}

int TaskManager::getWorkerCount() const {
    return static_cast<int>(workers.size());
}

void TaskManager::addWorker() {
    int id = generateRandomWorkerID();
    workers.emplace_back(std::make_unique<Worker>(id));
}

void TaskManager::assignTaskToWorker(int workerIndex, std::function<void()> task) {
    if (workerIndex >= 0 && workerIndex < static_cast<int>(workers.size())) {
        workers[workerIndex]->assignTask(std::move(task));
    }
}

int TaskManager::generateRandomWorkerID() {
    int id;
    do {
        id = dist(rng);
    } while (used_ids.find(id) != used_ids.end());
    used_ids.insert(id);
    return id;
}

void TaskManager::clearWorkers() {
    workers.clear();
    used_ids.clear();
}