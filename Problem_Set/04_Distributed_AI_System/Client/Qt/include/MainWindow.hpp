#pragma once

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <vector>
#include <memory>

class QPushButton;
class QProgressBar;
class QScrollArea;
class QVBoxLayout;
class QLabel;
class QWidget;
class ImageResultWidget;
class OCRClient;

// Main application window
class MainWindow : public QMainWindow {
  Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void onOCRResultReceived(const QString& imageId, const QString& text,
    const QByteArray& cleanedImage, bool success, const QString& error);
    void onConnectionError(const QString& error);

private slots:
    void onUploadClicked();
    void onProgressUpdated(int completed, int total);

private:
    void clearResults();
    void addImageWidget(const QString& imagePath, const QString& imageId);
    void updateStatusLabel();
    QString generateImageId();
    QString generateBatchId();

    QPushButton* uploadButton_;
    QProgressBar* progressBar_;
    QLabel* statusLabel_;
    QScrollArea* scrollArea_;
    QWidget* resultsContainer_;
    QVBoxLayout* resultsLayout_;
    
    std::vector<std::unique_ptr<ImageResultWidget>> imageWidgets_;
    std::unique_ptr<OCRClient> ocrClient_;
    
    QString currentBatchId_;
    int totalImages_;
    int completedImages_;
    int nextImageId_;
};