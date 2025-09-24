# pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

typedef std::vector<std::vector<double>> Matrix;

class Algorithm {
public:
	Algorithm() = default;
	~Algorithm() = default;

	void execute();

	Matrix stdMatrixMultiply(const Matrix& A, const Matrix& B);
	Matrix threadDotProduct(const Matrix& A, const Matrix& B);
	Matrix threadPerRow(const Matrix& A, const Matrix& B);
	Matrix threadLimitedWorkers(const Matrix& A, const Matrix& B);
private:
	double computeDotProduct(const Matrix& A, const Matrix& B, size_t i, size_t j);
};