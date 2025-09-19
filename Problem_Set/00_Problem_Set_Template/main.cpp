#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

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

    std::vector<std::vector<double>> matrixA;
    std::vector<std::vector<double>> matrixB;

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

    std::cout << "Matrix A:\n";
    for (const auto& row : matrixA) {
        for (double v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    std::cout << "\nMatrix B:\n";
    for (const auto& row : matrixB) {
        for (double v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    return 0;
}