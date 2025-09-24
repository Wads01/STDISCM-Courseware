#include "Algorithm.hpp"
#include "TaskManager.hpp"
#include <atomic>
#include <condition_variable>
#include <mutex>

void Algorithm::execute() {
    std::cout << "Algorithm task is being executed." << std::endl;
}

double Algorithm::computeDotProduct(const Matrix& A, const Matrix& B, size_t i, size_t j) {
    double sum = 0.0;
    for (size_t k = 0; k < A[0].size(); ++k) {
        sum += A[i][k] * B[k][j];
    }
    return sum;
}

Matrix Algorithm::stdMatrixMultiply(const Matrix& A, const Matrix& B) {
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

Matrix Algorithm::threadDotProduct(const Matrix& A, const Matrix& B) {
    if (A.empty() || B.empty() || A[0].size() != B.size()) {
        throw std::invalid_argument("Incompatible matrix dimensions for multiplication.");
    }
    size_t rowsA = A.size();
    size_t colsA = A[0].size();
    size_t colsB = B[0].size();
    Matrix result(rowsA, std::vector<double>(colsB, 0.0));

    TaskManager& manager = TaskManager::getInstance();

    // Clear all existing workers before starting new batch
    manager.clearWorkers();

    // 1. Create enough workers (one per dot product)
    size_t numWorkers = rowsA * colsB;
    for (size_t w = 0; w < numWorkers; ++w) {
        manager.addWorker();
    }

    // 2. Assign each dot product as a task to a worker
    size_t workerIndex = 0;
    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            auto task = [&, i, j]() {
                result[i][j] = computeDotProduct(A, B, i, j);
            };
            if (workerIndex < numWorkers) {
                manager.assignTaskToWorker(static_cast<int>(workerIndex), task);
            }
            ++workerIndex;
        }
    }

    return result;
}

Matrix Algorithm::threadPerRow(const Matrix& A, const Matrix& B) {
    if (A.empty() || B.empty() || A[0].size() != B.size()) {
        throw std::invalid_argument("Incompatible matrix dimensions for multiplication.");
    }
    size_t rowsA = A.size();
    size_t colsA = A[0].size();
    size_t colsB = B[0].size();
    Matrix result(rowsA, std::vector<double>(colsB, 0.0));

    TaskManager& manager = TaskManager::getInstance();

    // Clear all existing workers before starting new batch
    manager.clearWorkers();

    // 1. Create one worker per row
    for (size_t w = 0; w < rowsA; ++w) {
        manager.addWorker();
    }

    // 2. Assign each row as a task to a worker
    for (size_t i = 0; i < rowsA; ++i) {
        auto task = [&, i]() {
            for (size_t j = 0; j < colsB; ++j) {
                result[i][j] = computeDotProduct(A, B, i, j);
            }
        };
        if (i < rowsA) {
            manager.assignTaskToWorker(static_cast<int>(i), task);
        }
    }

    return result;
}

Matrix Algorithm::threadLimitedWorkers(const Matrix& A, const Matrix& B) {
    if (A.empty() || B.empty() || A[0].size() != B.size()) {
        throw std::invalid_argument("Incompatible matrix dimensions for multiplication.");
    }
    size_t rowsA = A.size();
    size_t colsB = B[0].size();
    Matrix result(rowsA, std::vector<double>(colsB, 0.0));

    TaskManager& manager = TaskManager::getInstance();

    // Clear all existing workers before starting new batch
    manager.clearWorkers();

    // 1. Create the workers
    int numWorkers = 16;
    for (int w = 0; w < numWorkers; ++w) {
        manager.addWorker();
    }

    // Synchronization primitives
    std::atomic<int> tasksRemaining(static_cast<int>(rowsA));
    std::mutex mtx;
    std::condition_variable cv;

    // 2. Assign rows to workers in a round-robin fashion
    for (int w = 0; w < numWorkers; ++w) {
        auto task = [&, w]() {
            for (size_t i = w; i < rowsA; i += numWorkers) {
                for (size_t j = 0; j < colsB; ++j) {
                    result[i][j] = computeDotProduct(A, B, i, j);
                }
                // Decrement the counter for each row completed
                if (--tasksRemaining == 0) {
                    std::lock_guard<std::mutex> lock(mtx);
                    cv.notify_one();
                }
            }
        };
        manager.assignTaskToWorker(w, task);
    }

    // Wait for all tasks to finish
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&tasksRemaining] { return tasksRemaining == 0; });
    }

    return result;
}