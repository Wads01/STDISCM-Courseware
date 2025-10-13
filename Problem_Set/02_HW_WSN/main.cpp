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
    bool hasTemp;
};

int main(int argc, char** argv) {
    float distanceThreshold = 300.0f;
    std::vector<Cell> cells;

    std::ifstream ifs(CONFIG_PATH);
    if (ifs) {
        std::string line;

        // Read first non-empty line
        std::string firstLine;
        while (std::getline(ifs, line)) {
            if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;
            firstLine = line;
            break;
        }

        if (!firstLine.empty()) {
            std::istringstream iss(firstLine);
            std::vector<std::string> tokens;
            std::string tok;
            while (iss >> tok) tokens.push_back(tok);

            if (tokens.size() == 1) {
                try {
                    distanceThreshold = std::stof(tokens[0]);
                }
                catch (...) {
                    distanceThreshold = 300.0f;
                }
            }
            else {
                std::istringstream issCell(firstLine);
                int x, y;
                float temp;
                if (issCell >> x >> y >> temp) {
                    cells.push_back({ x, y, temp, true });
                }
                else if (issCell.clear(), issCell.str(firstLine), (issCell >> x >> y)) {
                    cells.push_back({ x, y, 0.0f, false });
                }
            }
        }

        // Read remaining lines as cells (x y [temp])
        while (std::getline(ifs, line)) {
            if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;
            std::istringstream iss(line);
            int x, y;
            float temp;
            if (iss >> x >> y >> temp) {
                cells.push_back({ x, y, temp, true });
            }
            else {
                iss.clear();
                iss.str(line);
                if (iss >> x >> y) {
                    cells.push_back({ x, y, 0.0f, false });
                }
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
        if (c.hasTemp) temps.push_back(c.temp);
    }

    QApplication app(argc, argv);
    MainWindow w(sites, temps, distanceThreshold);
    w.show();

    return app.exec();
}