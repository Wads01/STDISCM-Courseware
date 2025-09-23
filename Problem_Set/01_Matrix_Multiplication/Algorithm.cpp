#include "Algorithm.hpp"
#include "TaskManager.hpp"

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

    // 1. Create enough workers (one per dot product)
    size_t numWorkers = rowsA * colsB;
    for (size_t w = 0; w < numWorkers; ++w) {
        manager.addWorker();
    }

    // 2. Assign each dot product as a task to a worker
    size_t workerIndex = 0;
    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            // Create a lambda that computes the dot product and stores it in result
            auto task = [&, i, j]() {
                result[i][j] = computeDotProduct(A, B, i, j);
            };
            // Assign the task to the worker
            manager.assignTaskToWorker(static_cast<int>(workerIndex), task);
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
        manager.assignTaskToWorker(static_cast<int>(i), task);
    }

    return result;
}