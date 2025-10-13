#pragma once
#include <QMainWindow>
#include <QPointF>
#include <vector>

class QPushButton;
class VoronoiWidget;
class QTimer;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const std::vector<QPointF>& sites, const std::vector<float>& temps, float distanceThreshold, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadConfig();
    void onPrintTemperaturesPeriodically();

private:
    void startSimulation();
    void stopSimulation();

    QPushButton* loadButton_;
    VoronoiWidget* canvas_;
    QTimer* printTimer_;
    
    static constexpr int PRINT_INTERVAL_MS = 2000;
};