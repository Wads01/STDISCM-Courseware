#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <thread>

#include "Worker.hpp"
#include "ThreadPool.hpp"

using Matrix = std::vector<std::vector<double>>;

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

    if (matrixA.empty() || matrixB.empty() || matrixA[0].size() != matrixB.size()) {
        throw std::invalid_argument("Incompatible matrix dimensions for multiplication.");
    }

    // ============================ Traditional Approach ============================

    auto std_start_time = std::chrono::steady_clock::now();

    // Create matrixC with correct dimensions initialized to 0
	matrixC.assign(rowsA, std::vector<double>(colsB, 0.0));

    matrixC = stdMatrixMultiply(matrixA, matrixB);

    auto std_end_time = std::chrono::steady_clock::now();
    auto std_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std_end_time - std_start_time).count();

    // =========================== Thread Per Row Approach ===================================

    //auto thread_start_time = std::chrono::steady_clock::now();

    //matrixD.assign(rowsA, std::vector<double>(colsB, 0.0));

    //// Vector of threads size rowsA
    //std::vector<std::thread> threads;
    //threads.reserve(rowsA);

    //// One thread per row
    //for (size_t i = 0; i < rowsA; ++i) {
    //    threads.emplace_back(Worker(matrixA, matrixB, matrixD, i, i + 1));
    //}

    //for (auto& t : threads) {
    //    if (t.joinable()) t.join();
    //}

    //auto thread_end_time = std::chrono::steady_clock::now();
    //auto thread_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(thread_end_time - thread_start_time).count();

    // ============================ ThreadPool Approach ===================================

    auto threadpool_start_time = std::chrono::steady_clock::now();

    matrixD.assign(rowsA, std::vector<double>(colsB, 0.0));

	// Determine number of hardware threads available
    unsigned int hc = std::thread::hardware_concurrency();
    size_t hwThreads = static_cast<size_t>(hc);

	// Get minimum of hardware threads and number of rows
    size_t numWorkers = std::min<size_t>(hwThreads, rowsA);
    {
        ThreadPool pool(numWorkers);
        for (size_t i = 0; i < rowsA; ++i) {
            pool.enqueue(Worker(matrixA, matrixB, matrixD, i, i + 1));
        }
    }

    auto threadpool_end_time = std::chrono::steady_clock::now();
    auto threadpool_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(threadpool_end_time - threadpool_start_time).count();

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
    outfile << "Multithreaded Running Time: " << threadpool_duration << " ns.\n";
    outfile.close();

    return 0;
}

Matrix stdMatrixMultiply(const Matrix& A, const Matrix& B) {
    size_t rowsA = A.size();
    size_t inner = A[0].size();
    size_t colsB = B[0].size();
    Matrix result(rowsA, std::vector<double>(colsB, 0.0));
    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {

            double sum = 0.0;

            for (size_t k = 0; k < inner; ++k) {
                sum += A[i][k] * B[k][j];
            }

            result[i][j] = sum;
        }
    }
    return result;
}