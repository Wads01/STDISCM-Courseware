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

#include <sstream>

MainWindow::MainWindow(const std::vector<QPointF>& sites, const std::vector<float>& temps, float distanceThreshold, QWidget* parent)
    : QMainWindow(parent)
    , loadButton_(nullptr)
    , canvas_(nullptr)
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

	// Pass Data to voronoi widget 
    canvas_->setData(sites, temps, distanceThreshold);

    // Debug
	canvas_->printNeighbors();

    connect(loadButton_, &QPushButton::clicked, this, &MainWindow::onLoadConfig);
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

    float distanceThreshold = 0.0f;
    std::vector<QPointF> sites;
    std::vector<float> temps;

    // Read first line as threshold
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        std::istringstream iss(line.toStdString());
        if (!(iss >> distanceThreshold)) {
            QMessageBox::warning(this, tr("Parse error"), tr("Could not parse distance threshold from first non-empty line"));
        }
        break;
    }

    // Read remaining lines as cells: x y temp
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
    }

    canvas_->setData(sites, temps, distanceThreshold);

    // Debug
	canvas_->printNeighbors();
}