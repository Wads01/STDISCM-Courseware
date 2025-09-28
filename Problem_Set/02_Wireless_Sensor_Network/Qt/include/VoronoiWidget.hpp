#pragma once

#include <QWidget>
#include <QPointF>
#include <vector>

class VoronoiWidget : public QWidget {
    Q_OBJECT
public:
    explicit VoronoiWidget(QWidget* parent = nullptr);

    void setData(const std::vector<QPointF>& sites, const std::vector<float>& temps, float distanceThreshold);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void regenerateImage();

    std::vector<QPointF> inputSites_;
    std::vector<QPointF> mappedSites_;
    std::vector<float> temps_;
    std::vector<QColor> colors_;
    float distanceThreshold_ = 0.0f;

    QImage cachedImage_;
};