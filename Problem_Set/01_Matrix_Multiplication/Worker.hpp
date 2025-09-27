#pragma once

#include <vector>
#include <cstddef>
#include <stdexcept>

class Worker {
public:
    using Matrix = std::vector<std::vector<double>>;

    Worker(const Matrix& A, const Matrix& B, Matrix& result, std::size_t startRow, std::size_t endRow);

    void operator()();

private:
    const Matrix& matrixA;
    const Matrix& matrixB;
    Matrix& matrixResult;
    std::size_t mStartRow;
    std::size_t mEndRow;
};