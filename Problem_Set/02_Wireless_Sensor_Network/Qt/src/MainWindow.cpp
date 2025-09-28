#include "MainWindow.hpp"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSizePolicy>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , loadButton_(nullptr)
    , canvas_(nullptr)
{
    setFixedSize(800, 600);

    // Build central widget with a top bar and a canvas area
    auto* central = new QWidget(this);
    auto* vlay = new QVBoxLayout(central);
    vlay->setContentsMargins(8, 8, 8, 8);
    vlay->setSpacing(8);

    // Top bar containing the centered "Load Config" button
    auto* topBar = new QWidget(central);
    auto* hlay = new QHBoxLayout(topBar);
    hlay->setContentsMargins(0, 0, 0, 0);
    hlay->setSpacing(0);

    loadButton_ = new QPushButton("Load Config", topBar);
    loadButton_->setFixedHeight(26);
    loadButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    hlay->addWidget(loadButton_);

    // Canvas placeholder (expands to fill remaining space)
    canvas_ = new QWidget(central);
    canvas_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    vlay->addWidget(topBar);
    vlay->addWidget(canvas_);

    setCentralWidget(central);

    // No-op for now; placeholder for future implementation
    connect(loadButton_, &QPushButton::clicked, this, []() {
        // intentionally empty
        });
}