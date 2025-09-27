#pragma once

#include "IWorker.hpp"
#include <cstddef>

class Worker : public IWorker {
public:
    using Matrix = std::vector<std::vector<double>>;

    Worker(const Matrix& A, const Matrix& B, Matrix& result, std::size_t startRow, std::size_t endRow);

    void operator()() override;

private:
    double computeDot(std::size_t i, std::size_t j) const;

    const Matrix& matrixA;
    const Matrix& matrixB;
    Matrix& matrixResult;
    std::size_t mStartRow;
    std::size_t mEndRow;
};