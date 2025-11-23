#include "MainWindow.hpp"
#include "ImageResultWidget.hpp"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QProgressBar>
#include <QLabel>
#include <QPixmap>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>

ImageResultWidget::ImageResultWidget(const QString& imagePath, const QString& imageId, QWidget* parent) : QWidget(parent)
    , imageId_(imageId)
{
    auto* layout = new QHBoxLayout(this);
    layout->setSpacing(10);
    
    // Image thumbnail
    imageLabel_ = new QLabel(this);
    QPixmap pixmap(imagePath);
    imageLabel_->setPixmap(pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imageLabel_->setFixedSize(150, 150);
    imageLabel_->setStyleSheet("border: 1px solid #555;");
 
    // Result text area
    auto* rightLayout = new QVBoxLayout();
  
    statusLabel_ = new QLabel("Processing...", this);
    statusLabel_->setStyleSheet("color: orange; font-weight: bold;");
    
    resultLabel_ = new QLabel("Waiting for result...", this);
    resultLabel_->setWordWrap(true);
    resultLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    resultLabel_->setMinimumHeight(100);
    resultLabel_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    resultLabel_->setStyleSheet("padding: 5px; background-color: #2b2b2b; border: 1px solid #555;");
    
    rightLayout->addWidget(statusLabel_);
    rightLayout->addWidget(resultLabel_);
    
    layout->addWidget(imageLabel_);
    layout->addLayout(rightLayout, 1);
    
    setStyleSheet("QWidget { border: 1px solid #555; border-radius: 5px; padding: 5px; }");
}

void ImageResultWidget::setResult(const QString& ocrText) {
    statusLabel_->setText("✓ Completed");
    statusLabel_->setStyleSheet("color: green; font-weight: bold;");
    resultLabel_->setText(ocrText.isEmpty() ? "(No text detected)" : ocrText);
}

void ImageResultWidget::setError(const QString& errorMsg) {
    statusLabel_->setText("✗ Error");
    statusLabel_->setStyleSheet("color: red; font-weight: bold;");
    resultLabel_->setText(errorMsg);
    resultLabel_->setStyleSheet("padding: 5px; background-color: #3d2020; color: #ff6666; border: 1px solid #aa3333;");
}


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
    , totalImages_(0)
    , completedImages_(0)
    , nextImageId_(1)
{
    setWindowTitle("Distributed OCR Client");
    setMinimumSize(900, 700);
    
    // Central widget
    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Upload button
    uploadButton_ = new QPushButton("Upload Images", this);
    uploadButton_->setFixedHeight(50);
    uploadButton_->setStyleSheet("font-size: 16pt; font-weight: bold;");
 
    // Progress bar
    progressBar_ = new QProgressBar(this);
    progressBar_->setTextVisible(false);
    progressBar_->setMinimum(0);
    progressBar_->setValue(0);
    progressBar_->setFixedHeight(15);
    progressBar_->setStyleSheet(
        "QProgressBar {"
            "   border: none;"
            "   background-color: #3a3a3a;"
            "   border-radius: 7px;"
        "}"
        "QProgressBar::chunk {"
            "   background-color: #8b5cf6;"
            "   border-radius: 7px;"
        "}"
    );
    
    // Status label
    statusLabel_ = new QLabel("0 / 0 images processed (0%)", this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setStyleSheet("font-size: 14pt; color: #aaa;");
  
    // Scroll area
    scrollArea_ = new QScrollArea(this);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setStyleSheet("QScrollArea { border: none; }");
    
    resultsContainer_ = new QWidget();
    resultsLayout_ = new QVBoxLayout(resultsContainer_);
    resultsLayout_->setSpacing(10);
    resultsLayout_->setAlignment(Qt::AlignTop);
    
    scrollArea_->setWidget(resultsContainer_);
    
    mainLayout->addWidget(uploadButton_);
    mainLayout->addWidget(progressBar_);
    mainLayout->addWidget(statusLabel_);
    mainLayout->addWidget(scrollArea_, 1);
    
    setCentralWidget(central);
    
    // Connect signals
    connect(uploadButton_, &QPushButton::clicked, this, &MainWindow::onUploadClicked);
    connect(this, &MainWindow::resultReceived, this, &MainWindow::onResultReceived);
}

MainWindow::~MainWindow() = default;

void MainWindow::onUploadClicked() {
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Upload Images",
        "Do you want to select a directory?\n\nYes = Select Directory\nNo = Select Files",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
    );
    
    QStringList filePaths;
    
    if (reply == QMessageBox::Yes) {
        QString dirPath = QFileDialog::getExistingDirectory(this,
            "Select Directory",
            QString(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
  
        if (!dirPath.isEmpty()) {
            QDir dir(dirPath);
            QStringList filters;
            filters << "*.jpg" << "*.jpeg" << "*.JPG" << "*.JPEG" 
            << "*.png" << "*.PNG";
         
            QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
            for (const QFileInfo& fileInfo : fileList) {
                filePaths.append(fileInfo.absoluteFilePath());
            }
            
            if (filePaths.isEmpty()) {
                QMessageBox::information(this, "No Images", "No .jpg or .png images found in the selected directory.");
                return;
            }
        }
    } else if (reply == QMessageBox::No) {
        filePaths = QFileDialog::getOpenFileNames(this,
            "Select Images",
            QString(),
            "Images (*.png *.PNG *.jpg *.jpeg *.JPG *.JPEG);;All Files (*)"
        );
    }

    if (filePaths.isEmpty()) return;
    
    // 100% - start new batch
    if (completedImages_ == totalImages_ && totalImages_ > 0) {
        clearResults();
    }
    
    // Generate new batch ID if starting fresh
    if (totalImages_ == 0) {
      currentBatchId_ = generateBatchId();
    }
    
    // Add images to UI
    for (const QString& path : filePaths) {
        QString imageId = generateImageId();
        addImageWidget(path, imageId);
    }
  
    totalImages_ += filePaths.size();
    progressBar_->setMaximum(totalImages_);
    progressBar_->setValue(completedImages_);
    
    updateStatusLabel();
    
    // TODO: Emit signal to gRPC client to upload images
    emit uploadImages(filePaths);
}

void MainWindow::onResultReceived(const QString& imageId, const QString& text, bool success, const QString& error) {
    // Find the widget with matching imageId
    for (auto& widget : imageWidgets_) {
        if (widget->getImageId() == imageId) {
            if (success) {
                widget->setResult(text);
            } else {
                widget->setError(error);
            }
            break;
        }
    }
    
    completedImages_++;
    progressBar_->setValue(completedImages_);
    updateStatusLabel();
}

void MainWindow::onProgressUpdated(int completed, int total) {
    completedImages_ = completed;
    totalImages_ = total;
    progressBar_->setMaximum(total);
    progressBar_->setValue(completed);
    updateStatusLabel();
}

void MainWindow::updateStatusLabel() {
    if (totalImages_ == 0) {
        statusLabel_->setText("0 / 0 images processed (0%)");
        statusLabel_->setStyleSheet("font-size: 14pt; color: #aaa;");
    } else {
        int percentage = (totalImages_ > 0) ? (completedImages_ * 100 / totalImages_) : 0;
        statusLabel_->setText(QString("%1 / %2 images processed (%3%)")
            .arg(completedImages_)
            .arg(totalImages_)
            .arg(percentage));
  
        if (completedImages_ == totalImages_) {
            statusLabel_->setStyleSheet("font-size: 14pt; color: #8b5cf6; font-weight: bold;");
        } else {
            statusLabel_->setStyleSheet("font-size: 14pt; color: #aaa;");
        }
    }
}

void MainWindow::clearResults() {
    imageWidgets_.clear();
    
    // Clear layout
    QLayoutItem* item;
    while ((item = resultsLayout_->takeAt(0)) != nullptr) {
    delete item->widget();
    delete item;
    }
  
    totalImages_ = 0;
    completedImages_ = 0;
    progressBar_->setValue(0);
    progressBar_->setMaximum(0);
    updateStatusLabel();
}

void MainWindow::addImageWidget(const QString& imagePath, const QString& imageId) {
    auto widget = std::make_unique<ImageResultWidget>(imagePath, imageId, resultsContainer_);
    resultsLayout_->addWidget(widget.get());
    imageWidgets_.push_back(std::move(widget));
}

QString MainWindow::generateImageId() {
    return QString("img_%1_%2").arg(currentBatchId_).arg(nextImageId_++);
}

QString MainWindow::generateBatchId() {
    nextImageId_ = 1; // Reset image counter for new batch
    return QString::number(QDateTime::currentMSecsSinceEpoch());
}