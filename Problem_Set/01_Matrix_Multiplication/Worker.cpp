#include "Worker.hpp"

Worker::Worker(const Matrix& A, const Matrix& B, Matrix& result, std::size_t startRow, std::size_t endRow)
    : matrixA(A), matrixB(B), matrixResult(result), mStartRow(startRow), mEndRow(endRow)
{
    if (A.empty() || B.empty() || A[0].size() != B.size()) {
        throw std::invalid_argument("Incompatible matrix dimensions for multiplication.");
    }
}

void Worker::operator()()
{
    std::size_t colsB = matrixB[0].size();
    for (std::size_t i = mStartRow; i < mEndRow; ++i) {
        for (std::size_t j = 0; j < colsB; ++j) {

            double sum = 0.0;

            for (std::size_t k = 0; k < matrixA[0].size(); ++k) {
				sum += matrixA[i][k] * matrixB[k][j];
            }

            matrixResult[i][j] = sum;
        }
    }
}