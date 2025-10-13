#pragma once
#include <QMainWindow>
#include <QPointF>
#include <vector>

class QPushButton;
class VoronoiWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const std::vector<QPointF>& sites, const std::vector<float>& temps, float distanceThreshold, QWidget* parent = nullptr);

private slots:
    void onLoadConfig();

private:
    QPushButton* loadButton_;
    VoronoiWidget* canvas_;
};