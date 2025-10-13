#pragma once

#include <QWidget>
#include <QPointF>
#include <vector>
#include <memory>
#include <atomic>

#include "Sensor.hpp"

using SensorPtr = std::unique_ptr<sim::Sensor>;

class VoronoiWidget : public QWidget {
    Q_OBJECT
public:
    explicit VoronoiWidget(QWidget* parent = nullptr);

    void setData(const std::vector<QPointF>& sites, const std::vector<float>& temps, float distanceThreshold);

    void printNeighbors() const;
    
    void startSensorSimulation(int sensorDelayMs = 200);
    void stopSensorSimulation();
    std::vector<float> getCurrentTemperatures() const;
    void updateDisplayTemperatures();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void regenerateImage();

    void computeNeighbors();
    void initializeSensors();

    void computeBoundingBox(double& minX, double& minY, double& maxX, double& maxY) const;
    void computeScaleAndOffset(int w, int h, double minX, double minY, double maxX, double maxY, double& outScale, double& outOffsetX, double& outOffsetY, int margin = 20) const;
    int nearestSiteIndex(int px, int py, double& outDistSq) const;
    void rasterize(int w, int h, double thresholdSq);

    std::vector<std::vector<int>> neighbors_;

    std::vector<QPointF> inputSites_;
    std::vector<QPointF> mappedSites_;
    std::vector<float> temps_;

    std::vector<QColor> colors_;
    float distanceThreshold_ = 0.0f;

    QImage cachedImage_;
    
    std::vector<SensorPtr> sensors_;
    std::shared_ptr<std::vector<std::atomic<float>>> sharedTemps_;
};