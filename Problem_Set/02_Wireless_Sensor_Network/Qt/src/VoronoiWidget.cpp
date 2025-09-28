#include "VoronoiWidget.hpp"

#include <QPainter>
#include <QImage>
#include <QColor>
#include <QTextOption>
#include <cmath>

VoronoiWidget::VoronoiWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void VoronoiWidget::setData(const std::vector<QPointF>& sites, const std::vector<float>& temps, float distanceThreshold)
{
    inputSites_ = sites;
    temps_ = temps;
    distanceThreshold_ = distanceThreshold;

    // generate deterministic colors per site
    colors_.clear();
    colors_.reserve(inputSites_.size());
    for (size_t i = 0; i < inputSites_.size(); ++i) {
        int hue = static_cast<int>((i * 97) % 360);
        QColor c = QColor::fromHsv(hue, 200, 220);
        colors_.push_back(c);
    }

    regenerateImage();
    update();
}

void VoronoiWidget::resizeEvent(QResizeEvent* /*event*/)
{
    regenerateImage();
}

void VoronoiWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(40, 40, 40)); // frame/background

    if (!cachedImage_.isNull()) {
        p.drawImage(0, 0, cachedImage_);
    }

    // Draw site markers and temperatures
    p.setRenderHint(QPainter::Antialiasing, true);
    QFont font = p.font();
    font.setBold(true);
    font.setPointSize(10);
    p.setFont(font);

    for (size_t i = 0; i < mappedSites_.size(); ++i) {
        const QPointF& pt = mappedSites_[i];
        QColor c = colors_[i];
        p.setBrush(c);
        p.setPen(Qt::NoPen);
        p.drawEllipse(pt, 4.0, 4.0);

        // Determine contrasting text color
        int brightness = (c.red() * 299 + c.green() * 587 + c.blue() * 114) / 1000;
        QColor textColor = (brightness < 128) ? Qt::white : Qt::black;
        p.setPen(textColor);

        if (i < temps_.size()) {
            QString txt = QString::number(temps_[i], 'f', 1);
            QRectF textRect(pt.x() + 6, pt.y() - 10, 60, 20);
            p.drawText(textRect, txt);
        }
    }
}

void VoronoiWidget::regenerateImage()
{
    const int w = width();
    const int h = height();
    if (w <= 0 || h <= 0) {
        cachedImage_ = QImage();
        return;
    }

    cachedImage_ = QImage(w, h, QImage::Format_RGB32);
    cachedImage_.fill(QColor(60, 60, 60)); // interior background

    if (inputSites_.empty()) return;

    // Compute bounding box of input sites
    double minX = inputSites_[0].x();
    double minY = inputSites_[0].y();
    double maxX = minX;
    double maxY = minY;
    for (const auto& s : inputSites_) {
        minX = std::min(minX, (double)s.x());
        minY = std::min(minY, (double)s.y());
        maxX = std::max(maxX, (double)s.x());
        maxY = std::max(maxY, (double)s.y());
    }

    double bw = maxX - minX;
    double bh = maxY - minY;
    const int margin = 20;

    double availW = std::max(1.0, double(w - 2 * margin));
    double availH = std::max(1.0, double(h - 2 * margin));
    double scaleX = bw > 0.0 ? availW / bw : 1.0;
    double scaleY = bh > 0.0 ? availH / bh : 1.0;
    double scale = std::min(scaleX, scaleY);

    // Center offset if aspect ratio leaves extra space
    double usedW = (bw > 0.0 ? bw * scale : 0.0);
    double usedH = (bh > 0.0 ? bh * scale : 0.0);
    double offsetX = margin + (availW - usedW) * 0.5 - minX * scale;
    double offsetY = margin + (availH - usedH) * 0.5 - minY * scale;

    // Map input sites to widget coords
    mappedSites_.clear();
    mappedSites_.reserve(inputSites_.size());
    for (const auto& s : inputSites_) {
        QPointF m(s.x() * scale + offsetX, s.y() * scale + offsetY);
        mappedSites_.push_back(m);
    }

    // Precalculate threshold in pixel-space
    double thresholdInPixels = distanceThreshold_ * scale;
    double thresholdSq = thresholdInPixels * thresholdInPixels;

    // For each pixel, determine nearest site
    for (int y = 0; y < h; ++y) {
        QRgb* scanLine = reinterpret_cast<QRgb*>(cachedImage_.scanLine(y));
        for (int x = 0; x < w; ++x) {
            double bestDistSq = std::numeric_limits<double>::infinity();
            int bestIdx = -1;
            for (size_t i = 0; i < mappedSites_.size(); ++i) {
                double dx = mappedSites_[i].x() - x;
                double dy = mappedSites_[i].y() - y;
                double d2 = dx * dx + dy * dy;
                if (d2 < bestDistSq) {
                    bestDistSq = d2;
                    bestIdx = static_cast<int>(i);
                }
            }

            QColor color;
            if (bestIdx >= 0) {
                if (distanceThreshold_ > 0.0 && bestDistSq > thresholdSq) {
                    // outside threshold => darker background
                    color = QColor(80, 80, 80);
                }
                else {
                    color = colors_[bestIdx];
                }
            }
            else {
                color = QColor(80, 80, 80);
            }

            scanLine[x] = color.rgb();
        }
    }
}