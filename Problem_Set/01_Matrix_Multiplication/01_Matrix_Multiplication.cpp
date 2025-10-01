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

static std::string getDefaultInputPath();
static std::string getDefaultOutputPath();
void stdMatrixMultiply(const Matrix& A, const Matrix& B, Matrix& result, size_t rowsA, size_t colsB);
void roundRobinWorker(const Matrix& A, const Matrix& B, Matrix& result, size_t start, size_t stride, size_t rowsA, size_t colsB);

int main(int argc, char* argv[]) {
    std::string infilePath;
    if (argc == 2) {
        infilePath = argv[1];
    }
    else {
        infilePath = getDefaultInputPath();
    }

    std::ifstream infile(infilePath);
    if (!infile) {
        std::cerr << "Error: Could not open " << infilePath << '\n';
        return 1;
    }


    Matrix matrixA, matrixB, matrixC, matrixD, matrixE, matrixF;

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
        std::cout << ("Incompatible matrix dimensions for multiplication.") << std::endl;

		std::ofstream outfile(getDefaultOutputPath());
		outfile << "Incompatible matrix dimensions for multiplication.\n";
		outfile.close();

		return 1;
    }

    // Create matrices with correct dimensions initialized to 0
    matrixC.assign(rowsA, std::vector<double>(colsB, 0.0));
    matrixD.assign(rowsA, std::vector<double>(colsB, 0.0));
    matrixE.assign(rowsA, std::vector<double>(colsB, 0.0));
    matrixF.assign(rowsA, std::vector<double>(colsB, 0.0));

    // Determine number of hardware threads available
    unsigned int hc = std::thread::hardware_concurrency();
    size_t hwThreads = static_cast<size_t>(hc);

    // Get minimum of hardware threads and number of rows
    size_t numWorkers = std::min<size_t>(hwThreads, rowsA);

    // ============================ Traditional Approach ============================

    auto std_start_time = std::chrono::steady_clock::now();

    stdMatrixMultiply(std::cref(matrixA), std::cref(matrixB), std::ref(matrixC), rowsA, colsB);

    auto std_end_time = std::chrono::steady_clock::now();
    auto std_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std_end_time - std_start_time).count();

    // =========================== Thread Per Row Approach ===================================

    //auto thread_start_time = std::chrono::steady_clock::now();

    //std::vector<std::thread> threads;
    //threads.reserve(rowsA);

    //for (size_t i = 0; i < rowsA; ++i)
    //    threads.emplace_back(Worker(matrixA, matrixB, matrixD, i, i + 1));

    //for (auto& t1 : threads)
    //    if (t1.joinable()) t1.join();

    //auto thread_end_time = std::chrono::steady_clock::now();
    //auto thread_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(thread_end_time - thread_start_time).count();

    // ============================ Round Robin Approach ===================================

	auto rr_start_time = std::chrono::steady_clock::now();

    std::vector<std::thread> rrThreads;
    rrThreads.reserve(numWorkers);

    for (size_t t = 0; t < numWorkers; ++t) {
        rrThreads.emplace_back(roundRobinWorker,
            std::cref(matrixA), std::cref(matrixB), std::ref(matrixE), t, numWorkers, rowsA, colsB);
    }

    for (auto& t2 : rrThreads)
        if (t2.joinable()) t2.join();

    auto rr_end_time = std::chrono::steady_clock::now();
    auto rr_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(rr_end_time - rr_start_time).count();

    // ============================ ThreadPool Approach ===================================

    //auto threadpool_start_time = std::chrono::steady_clock::now();

    //{
    //    ThreadPool pool(numWorkers);
    //    for (size_t i = 0; i < rowsA; ++i)
    //        pool.enqueue(Worker(matrixA, matrixB, matrixF, i, i + 1));
    //}

    //auto threadpool_end_time = std::chrono::steady_clock::now();
    //auto threadpool_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(threadpool_end_time - threadpool_start_time).count();

    // ============================ Output Results ==================================

    std::ofstream outfile(getDefaultOutputPath());
    outfile << std::fixed << std::setprecision(1);
    for (const auto& row : matrixE) {
        for (const auto& val : row) {
            outfile << val << " ";
        }
        outfile << "\n";
    }
    outfile << "\n";
    outfile << "Standard Running Time: " << std_duration << " ns\n";
	//outfile << "Thread Per Row Running Time: " << thread_duration << " ns\n";
	//outfile << "Round Robin Running Time: " << rr_duration << " ns\n";
    //outfile << "Threadpool Running Time: " << threadpool_duration << " ns\n";
    outfile << "Multithread Running Time: " << rr_duration << " ns\n";

    outfile.close();

    return 0;
}

static std::string getDefaultInputPath() {
    std::string filePath(__FILE__);
    size_t pos = filePath.find_last_of("\\/");
    std::string dir = (pos == std::string::npos) ? std::string(".") : filePath.substr(0, pos);
    return dir + "/input.txt";
}

static std::string getDefaultOutputPath() {
    std::string filePath(__FILE__);
    size_t pos = filePath.find_last_of("\\/");
    std::string dir = (pos == std::string::npos) ? std::string(".") : filePath.substr(0, pos);
    return dir + "/output.txt";
}

void stdMatrixMultiply(const Matrix& A, const Matrix& B, Matrix& result, size_t rowsA, size_t colsB) {
    size_t innerA = A[0].size();

    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < innerA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
}

void roundRobinWorker(const Matrix& A, const Matrix& B, Matrix& result, size_t start, size_t stride, size_t rowsA, size_t colsB) {
    size_t innerA = A[0].size();

    for (size_t i = start; i < rowsA; i += stride) {
        for (size_t j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < innerA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
}