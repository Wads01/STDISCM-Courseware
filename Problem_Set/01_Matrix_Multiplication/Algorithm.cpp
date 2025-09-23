#include "Algorithm.hpp"
#include "TaskManager.hpp"

void Algorithm::execute() {
    std::cout << "Algorithm task is being executed." << std::endl;
}

void Algorithm::createWorker() {
    TaskManager& manager = TaskManager::getInstance();
    manager.addWorker();
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

Matrix Algorithm::threadMatrixMultiply(const Matrix& A, const Matrix& B) {
    if (A.empty() || B.empty() || A[0].size() != B.size()) {
        throw std::invalid_argument("Incompatible matrix dimensions for multiplication.");
    }
    size_t rowsA = A.size();
    size_t colsA = A[0].size();
    size_t colsB = B[0].size();
    Matrix result(rowsA, std::vector<double>(colsB, 0.0));


    return { {0} };
}