#pragma once

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <vector>
#include <memory>

class QPushButton;
class QProgressBar;
class QScrollArea;
class QVBoxLayout;
class QLabel;
class QWidget;
class ImageResultWidget;

// Main application window
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void uploadImages(const QStringList& imagePaths);
    void resultReceived(const QString& imageId, const QString& text, bool success, const QString& error);

private slots:
    void onUploadClicked();
    void onResultReceived(const QString& imageId, const QString& text, bool success, const QString& error);
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
    
    QString currentBatchId_;
    int totalImages_;
    int completedImages_;
    int nextImageId_;
};