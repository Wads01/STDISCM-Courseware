#include "MainWindow.hpp"
#include <QApplication>
#include <QPointF>
#include <vector>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Initialize with empty data for now
    std::vector<QPointF> sites;
    std::vector<float> temps;
    float distanceThreshold = 300.0f;

    MainWindow window(sites, temps, distanceThreshold);
    window.setWindowTitle("Distributed AI System - Client");
    window.show();

    return app.exec();
}
