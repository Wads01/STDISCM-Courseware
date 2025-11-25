#include "MainWindow.hpp"
#include "ImageResultWidget.hpp"
#include "OCRClient.hpp"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QProgressBar>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QDateTime>
#include <QDir>
#include <QBuffer>

ImageResultWidget::ImageResultWidget(const QString& imagePath, const QString& imageId, QWidget* parent)
	: QWidget(parent)
	, imageId_(imageId)
{
	auto* layout = new QVBoxLayout(this);
	layout->setSpacing(5);
	layout->setContentsMargins(5, 5, 5, 5);
	
	imageLabel_ = new QLabel(this);
	imageLabel_->setFixedSize(180, 120);
	imageLabel_->setStyleSheet("background-color: white; border: 1px solid #555;");
	imageLabel_->setAlignment(Qt::AlignCenter);
	imageLabel_->setScaledContents(false);
	
	// Load and display original image as placeholder
	QPixmap pixmap(imagePath);
	if (!pixmap.isNull())
		imageLabel_->setPixmap(pixmap.scaled(180, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	else
		imageLabel_->setText("Loading...");
	
	statusLabel_ = new QLabel("In progress", this);
	statusLabel_->setStyleSheet("color: #888; font-size: 10pt;");
	statusLabel_->setAlignment(Qt::AlignCenter);
	
	resultLabel_ = new QLabel("Processing...", this);
	resultLabel_->setWordWrap(true);
	resultLabel_->setAlignment(Qt::AlignCenter);
	resultLabel_->setStyleSheet(
		"background-color: #2b2b2b; "
		"color: white; "
		"padding: 8px; "
		"font-size: 11pt; "
		"border: 1px solid #555;"
	);
	resultLabel_->setMinimumHeight(40);
	resultLabel_->setMaximumHeight(60);
	
	layout->addWidget(imageLabel_);
	layout->addWidget(statusLabel_);
	layout->addWidget(resultLabel_);
	
	setStyleSheet("QWidget { background-color: #3a3a3a; border-radius: 3px; }");
}

void ImageResultWidget::setResult(const QString& ocrText, const QByteArray& cleanedImageData) {
	statusLabel_->setText("");
	statusLabel_->hide();
	
	if (!cleanedImageData.isEmpty()) {
		QPixmap cleanedPixmap;
		cleanedPixmap.loadFromData(cleanedImageData);
		
		if (!cleanedPixmap.isNull())
			imageLabel_->setPixmap(cleanedPixmap.scaled(180, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
	
	if (ocrText.isEmpty()) {
		resultLabel_->setText("(No text detected)");
		resultLabel_->setStyleSheet(
			"background-color: #2b2b2b; "
			"color: #888; "
			"padding: 8px; "
			"font-size: 11pt; "
			"font-style: italic; "
			"border: 1px solid #555;"
		);
	} else {
		resultLabel_->setText(ocrText);
		resultLabel_->setStyleSheet(
			"background-color: #2b2b2b; "
			"color: white; "
			"padding: 8px; "
			"font-size: 11pt; "
			"border: 1px solid #555;"
		);
	}
}

void ImageResultWidget::setError(const QString& errorMsg) {
	statusLabel_->setText("Error");
	statusLabel_->setStyleSheet("color: #ff6666; font-size: 10pt; font-weight: bold;");
	statusLabel_->show();
	
	resultLabel_->setText(errorMsg);
	resultLabel_->setStyleSheet(
		"background-color: #3d2020; "
		"color: #ff6666; "
		"padding: 8px; "
		"font-size: 10pt; "
		"border: 1px solid #aa3333;"
	);
	
	imageLabel_->setStyleSheet("background-color: #3d2020; border: 1px solid #aa3333;");
}


MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, totalImages_(0)
	, completedImages_(0)
	, nextImageId_(1)
{
	setWindowTitle("Distributed OCR Client");
	setMinimumSize(900, 700);
	
	// Initialize gRPC client
	ocrClient_ = std::make_unique<OCRClient>("localhost:50051", this);
	connect(ocrClient_.get(), &OCRClient::resultReceived, this, &MainWindow::onOCRResultReceived);
	connect(ocrClient_.get(), &OCRClient::connectionError, this, &MainWindow::onConnectionError);
	
	// Central widget
	auto* central = new QWidget(this);
	auto* mainLayout = new QVBoxLayout(central);
	mainLayout->setSpacing(10);
	mainLayout->setContentsMargins(10, 10, 10, 10);
	
	uploadButton_ = new QPushButton("Upload Images", this);
	uploadButton_->setFixedHeight(50);
	uploadButton_->setStyleSheet("font-size: 16pt; font-weight: bold;");
	
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
		" background-color: #8b5cf6;"
		"   border-radius: 7px;"
		"}"
	);
	
	statusLabel_ = new QLabel("0 / 0 images processed (0%)", this);
	statusLabel_->setAlignment(Qt::AlignCenter);
	statusLabel_->setStyleSheet("font-size: 14pt; color: #aaa;");
	
	scrollArea_ = new QScrollArea(this);
	scrollArea_->setWidgetResizable(true);
	scrollArea_->setStyleSheet("QScrollArea { border: none; background-color: #2b2b2b; }");
	
	resultsContainer_ = new QWidget();
	resultsContainer_->setStyleSheet("background-color: #2b2b2b;");
	resultsLayout_ = new QGridLayout(resultsContainer_);
	resultsLayout_->setSpacing(15);
	resultsLayout_->setContentsMargins(10, 10, 10, 10);
	resultsLayout_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	
	scrollArea_->setWidget(resultsContainer_);
	
	mainLayout->addWidget(uploadButton_);
	mainLayout->addWidget(progressBar_);
	mainLayout->addWidget(statusLabel_);
	mainLayout->addWidget(scrollArea_, 1);
	
	setCentralWidget(central);
	
	// Connect signals
	connect(uploadButton_, &QPushButton::clicked, this, &MainWindow::onUploadClicked);
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
			filters << "*.jpg" << "*.jpeg" << "*.JPG" << "*.JPEG" << "*.png" << "*.PNG";
			
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
	
	// Upload to server via gRPC
	ocrClient_->uploadImages(filePaths, currentBatchId_);
}

void MainWindow::onOCRResultReceived(const QString& imageId, const QString& text, const QByteArray& cleanedImage, bool success, const QString& error) {
	// Find the widget with matching imageId and update with cleaned image
	for (auto& widget : imageWidgets_) {
		if (widget->getImageId() == imageId) {
			if (success)
				widget->setResult(text, cleanedImage);
			else
				widget->setError(error);
			break;
		}
	}
	
	completedImages_++;
	progressBar_->setValue(completedImages_);
	updateStatusLabel();
}

void MainWindow::onConnectionError(const QString& error) {
	QMessageBox::critical(this, "Connection Error",
	QString("Failed to connect to OCR server:\n%1").arg(error));
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
			.arg(percentage)
		);
		
		if (completedImages_ == totalImages_) {
			statusLabel_->setStyleSheet("font-size: 14pt; color: #8b5cf6; font-weight: bold;");
		} else {
			statusLabel_->setStyleSheet("font-size: 14pt; color: #aaa;");
		}
	}
}

void MainWindow::clearResults() {
	imageWidgets_.clear();
	
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
	
	int index = imageWidgets_.size();
	int row = index / 4;
	int col = index % 4;
	
	resultsLayout_->addWidget(widget.get(), row, col);
	imageWidgets_.push_back(std::move(widget));
}

QString MainWindow::generateImageId() {
	return QString("img_%1_%2").arg(currentBatchId_).arg(nextImageId_++);
}

QString MainWindow::generateBatchId() {
	nextImageId_ = 1;
	return QString::number(QDateTime::currentMSecsSinceEpoch());
}