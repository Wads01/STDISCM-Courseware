#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_set>
#include <random>

#include "TaskManager.hpp"

int main(int argc, char* argv[]) {
    std::string filepath = "input.txt";
    if (argc == 2) {
        filepath = argv[1];
    }

    std::ifstream infile("input.txt");
    if (!infile) {
        std::cerr << "Error: Could not open input.txt\n";
        return 1;   
    }
    
    TaskManager& manager = TaskManager::getInstance();

    manager.addWorker();

    Algorithm algo;
    manager.assignTaskToWorker(0, algo);

    infile.close();

    return 0;
}