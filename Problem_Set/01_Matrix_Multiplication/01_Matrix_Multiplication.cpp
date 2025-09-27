#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <thread>

#include "Worker.hpp"

using Matrix = std::vector<std::vector<double>>;

double computeDotProduct(const Matrix& A, const Matrix& B, size_t i, size_t j);
Matrix stdMatrixMultiply(const Matrix& A, const Matrix& B);

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

    Matrix matrixA, matrixB, matrixC, matrixD;

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

    size_t rowsA = matrixA.size();
    size_t colsB = matrixB[0].size();

    // ============================ Traditional Approach ============================

    // Start Time
    auto std_start_time = std::chrono::steady_clock::now();

    // Perform Matrix Multiplication
    matrixC = stdMatrixMultiply(matrixA, matrixB);

    // End Time
    auto std_end_time = std::chrono::steady_clock::now();

    // Calculate Duration
    auto std_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std_end_time - std_start_time).count();

    // =========================== Thread Per Row Approach ===================================

    auto thread_start_time = std::chrono::steady_clock::now();

    if (matrixA.empty() || matrixB.empty() || matrixA[0].size() != matrixB.size()) {
        throw std::invalid_argument("Incompatible matrix dimensions for multiplication.");
    }
	// Create matrixD with correct dimensions initialized to 0
    matrixD.assign(rowsA, std::vector<double>(colsB, 0.0));

	// Vector of threads size rowsA
    std::vector<std::thread> threads;
    threads.reserve(rowsA);
    
    // Launch one thread per row. Each Worker will compute a single row [i, i+1).
    for (size_t i = 0; i < rowsA; ++i) {
        threads.emplace_back(Worker(matrixA, matrixB, matrixD, i, i + 1));
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }


    auto thread_end_time = std::chrono::steady_clock::now();
    auto thread_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(thread_end_time - thread_start_time).count();

	// ============================ ThreadPool Approach ===================================

    auto threadpool_start_time = std::chrono::steady_clock::now();

    if (matrixA.empty() || matrixB.empty() || matrixA[0].size() != matrixB.size()) {
        throw std::invalid_argument("Incompatible matrix dimensions for multiplication.");
    }

    matrixD.assign(rowsA, std::vector<double>(colsB, 0.0));

    // TODO

    auto threadpool_end_time = std::chrono::steady_clock::now();
    auto threadpool_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(thread_end_time - thread_start_time).count();

    // ============================ Output Results ==================================

    std::ofstream outfile("output.txt");
    outfile << std::fixed << std::setprecision(1);
    for (const auto& row : matrixD) {
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

double computeDotProduct(const Matrix& A, const Matrix& B, size_t i, size_t j) {
    double sum = 0.0;
    for (size_t k = 0; k < A[0].size(); ++k) {
        sum += A[i][k] * B[k][j];
    }
    return sum;
}

Matrix stdMatrixMultiply(const Matrix& A, const Matrix& B) {
    if (A.empty() || B.empty() || A[0].size() != B.size()) {
        throw std::invalid_argument("Incompatible matrix dimensions for multiplication.");
    }
    size_t rowsA = A.size();
    size_t colsA = A[0].size();
    size_t colsB = B[0].size();
    Matrix result(rowsA, std::vector<double>(colsB, 0.0));
    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            result[i][j] = computeDotProduct(A, B, i, j);
        }
    }
    return result;
}
