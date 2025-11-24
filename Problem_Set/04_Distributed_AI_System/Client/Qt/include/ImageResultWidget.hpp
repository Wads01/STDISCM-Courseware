#pragma once
#include <QWidget>
#include <QString>
#include <QByteArray>

class QLabel;

class ImageResultWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImageResultWidget(const QString& imagePath, const QString& imageId, QWidget* parent = nullptr);
 
    void setResult(const QString& ocrText, const QByteArray& cleanedImageData);
    void setError(const QString& errorMsg);
    QString getImageId() const { return imageId_; }
    
private:
    QString imageId_;
    QLabel* imageLabel_;
    QLabel* cleanedImageLabel_;
    QLabel* resultLabel_;
    QLabel* statusLabel_;
};
