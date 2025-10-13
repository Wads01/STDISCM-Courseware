#include "MainWindow.hpp"
#include "VoronoiWidget.hpp"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QTimer>

#include <sstream>
#include <iostream>
#include <iomanip>

MainWindow::MainWindow(const std::vector<QPointF>& sites, const std::vector<float>& temps, float distanceThreshold, QWidget* parent)
    : QMainWindow(parent)
    , loadButton_(nullptr)
    , canvas_(nullptr)
    , printTimer_(nullptr)
{
    setFixedSize(800, 600);

    // Central widget
    auto* central = new QWidget(this);
    auto* vlay = new QVBoxLayout(central);
    vlay->setContentsMargins(8, 8, 8, 8);
    vlay->setSpacing(8);

    // Top bar button
    auto* topBar = new QWidget(central);
    auto* hlay = new QHBoxLayout(topBar);
    hlay->setContentsMargins(0, 0, 0, 0);
    hlay->setSpacing(0);

    loadButton_ = new QPushButton("Load Config", topBar);
    loadButton_->setFixedHeight(26);
    loadButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    hlay->addWidget(loadButton_);

    // Canvas (Voronoi)
    canvas_ = new VoronoiWidget(central);
    canvas_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    vlay->addWidget(topBar);
    vlay->addWidget(canvas_);

    setCentralWidget(central);

    printTimer_ = new QTimer(this);
    connect(printTimer_, &QTimer::timeout, this, &MainWindow::onPrintTemperaturesPeriodically);

    // Pass Data to voronoi widget 
    canvas_->setData(sites, temps, distanceThreshold);

    // Debug
    canvas_->printNeighbors();

    startSimulation();

    connect(loadButton_, &QPushButton::clicked, this, &MainWindow::onLoadConfig);
}

MainWindow::~MainWindow() {
    stopSimulation();
}

void MainWindow::onLoadConfig()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open config file"), QString(), tr("Text Files (*.txt);;All Files (*)"));
    if (path.isEmpty()) return;

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Open failed"), tr("Could not open file"));
        return;
    }

    QTextStream in(&f);
    QString line;

    float distanceThreshold = 300.0f;
    std::vector<QPointF> sites;
    std::vector<float> temps;

    // Read first non-empty line
    QString firstLine;
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        firstLine = line;
        break;
    }

    if (!firstLine.isEmpty()) {
        std::istringstream iss(firstLine.toStdString());
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
            std::istringstream issCell(firstLine.toStdString());
            int x, y;
            float temp;
            if (issCell >> x >> y >> temp) {
                sites.emplace_back(x, y);
                temps.push_back(temp);
            }
            else {
                issCell.clear();
                issCell.str(firstLine.toStdString());
                if (issCell >> x >> y) {
                    sites.emplace_back(x, y);
                }
            }
        }
    }

    // Read remaining lines as cells: x y [temp]
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        std::istringstream iss(line.toStdString());
        int x, y;
        float temp;
        if (iss >> x >> y >> temp) {
            sites.emplace_back(x, y);
            temps.push_back(temp);
        }
        else {
            iss.clear();
            iss.str(line.toStdString());
            if (iss >> x >> y) {
                sites.emplace_back(x, y);
            }
        }
    }

    canvas_->setData(sites, temps, distanceThreshold);

    // Debug
    canvas_->printNeighbors();
    
    stopSimulation();
    startSimulation();
}

void MainWindow::startSimulation() {
    std::cout << "Starting simulation...\n";
    canvas_->startSensorSimulation();
    printTimer_->start(PRINT_INTERVAL_MS);
}

void MainWindow::stopSimulation() {  
    std::cout << "Stopping simulation...\n";
    printTimer_->stop();
    canvas_->stopSensorSimulation();
}

void MainWindow::onPrintTemperaturesPeriodically() {
    auto temps = canvas_->getCurrentTemperatures();

    std::cout << "Current temperatures: ";
    for (size_t i = 0; i < temps.size(); ++i) {
        std::cout << std::fixed << std::setprecision(1) << temps[i];
        if (i < temps.size() - 1) std::cout << ", ";
    }
    std::cout << "\n";
}