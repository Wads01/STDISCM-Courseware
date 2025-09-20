#include "Algorithm.hpp"
#include "TaskManager.hpp"

void Algorithm::execute() {
    std::cout << "Algorithm task is being executed." << std::endl;
}

void Algorithm::createWorker() {
    TaskManager& manager = TaskManager::getInstance();
    manager.addWorker();
}