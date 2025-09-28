#include <QApplication>

#include "MainWindow.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>

struct Cell {
    int x;
    int y;
    float temp;
};

int main(int argc, char** argv) {
    float distanceThreshold = 0.0f;
    std::vector<Cell> cells;

    std::ifstream ifs(CONFIG_PATH);
    if (ifs) {
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;
            std::istringstream iss(line);
            if (!(iss >> distanceThreshold)) {
                std::cerr << "Could not parse distance threshold\n";
            }
            break;
        }

        while (std::getline(ifs, line)) {
            if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;
            std::istringstream iss(line);
            int x, y;
            float temp;
            if (iss >> x >> y >> temp) {
                cells.push_back({x, y, temp});
            }
        }
    }
    else {
        std::cerr << "Could not open config.txt\n";
        return 1;
    }

    std::vector<QPointF> sites;
    std::vector<float> temps;
    sites.reserve(cells.size());
    temps.reserve(cells.size());
    for (const auto& c : cells) {
        sites.emplace_back(static_cast<double>(c.x), static_cast<double>(c.y));
        temps.push_back(c.temp);
    }

    std::cout << "Distance Threshold: " << distanceThreshold << "\n";
    std::cout << "Cells:\n";
    for (const auto& cell : cells) {
        std::cout << "  (" << cell.x << ", " << cell.y << ") -> " << cell.temp << "\n";
    }

    QApplication app(argc, argv);
    MainWindow w(sites, temps, distanceThreshold);
    w.show();

    return app.exec();
}
