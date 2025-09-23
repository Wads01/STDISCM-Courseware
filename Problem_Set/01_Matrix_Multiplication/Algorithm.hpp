# pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

typedef std::vector<std::vector<double>> Matrix;

class Algorithm {
public:
	Algorithm() = default;
	~Algorithm() = default;

	void execute();

	Matrix stdMatrixMultiply(const Matrix& A, const Matrix& B);
	Matrix threadMatrixMultiply(const Matrix& A, const Matrix& B);
private:
	void createWorker();
	double computeDotProduct(const Matrix& A, const Matrix& B, size_t i, size_t j);
};