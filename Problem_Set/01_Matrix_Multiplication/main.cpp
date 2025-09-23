#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_set>
#include <random>
#include <chrono>
#include <iomanip>

#include "TaskManager.hpp"

typedef std::vector<std::vector<double>> Matrix;

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
    
    Matrix matrixA;
    Matrix matrixB;

    std::string line;
    bool readingFirst = true;

    while (std::getline(infile, line)) {
        if (line.empty()) {
            readingFirst = false;
            continue;
        }

        std::istringstream iss(line);
        std::vector<double> row;
        double val;

        while (iss >> val) {
            row.push_back(val);
        }

        if (readingFirst) {
            matrixA.push_back(row);
        }
        else {
            matrixB.push_back(row);
        }
    }
   
    infile.close();

	TaskManager& manager = TaskManager::getInstance();

    Algorithm algo;

    
    // Start Time
    auto std_start_time = std::chrono::steady_clock::now();

    // Perform Matrix Multiplication
	Matrix matrixC = algo.stdMatrixMultiply(matrixA, matrixB);

    // End Time
	auto std_end_time = std::chrono::steady_clock::now();

	// Calculate Duration
	auto std_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std_end_time - std_start_time).count();

    // =============================================================================================================

	auto thread_start_time = std::chrono::steady_clock::now();

	Matrix matrixD = algo.threadMatrixMultiply(matrixA, matrixB);

	auto thread_end_time = std::chrono::steady_clock::now();

	auto thread_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(thread_end_time - thread_start_time).count();

    // Write results to output.txt
    std::ofstream outfile("output.txt");
    outfile << std::fixed << std::setprecision(1);
    for (const auto& row : matrixC) {
        for (const auto& val : row) {
            outfile << val << " ";
        }
        outfile << "\n";
    }
    outfile << "\n";
    outfile << "Standard Running Time: " << std_duration << " ns.\n";
    outfile << "Multithreaded Running Time: " << thread_duration << " ns.\n";
    outfile.close();

    return 0;
}