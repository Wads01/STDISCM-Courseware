#include "OCRClient.hpp"
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <iostream>
#include <fstream>

OCRClient::OCRClient(const QString& server_address, QObject* parent) : QObject(parent)
    , server_address_(server_address)
{
    auto channel = grpc::CreateChannel(server_address.toStdString(), 
    grpc::InsecureChannelCredentials());
    stub_ = ocr::OCRService::NewStub(channel);
    
    std::cout << "OCR Client initialized for server: " << server_address.toStdString() << std::endl;
}

OCRClient::~OCRClient() = default;

void OCRClient::uploadImages(const QStringList& imagePaths, const QString& batchId) {
    // Process in a separate thread to avoid blocking Qt event loop
    std::thread([this, imagePaths, batchId]() {
        processImages(imagePaths, batchId);
    }).detach();
}

void OCRClient::processImages(const QStringList& imagePaths, const QString& batchId) {
    grpc::ClientContext context;
    auto stream = stub_->ProcessImages(&context);
    
    if (!stream) {
        emit connectionError("Failed to create gRPC stream");
        return;
    }
    
    // Start a thread to read results
    std::thread reader_thread([this, &stream]() {
        ocr::OCRResult result;
        while (stream->Read(&result)) {
            QString imageId = QString::fromStdString(result.image_id());
            QString text = QString::fromStdString(result.extracted_text());
            bool success = result.success();
            QString error = QString::fromStdString(result.error_message());
            
            // Convert cleaned image bytes to QByteArray
            QByteArray cleanedImage;
            if (success && result.cleaned_image_data().size() > 0) {
                cleanedImage = QByteArray(result.cleaned_image_data().data(), 
                result.cleaned_image_data().size());
            }
          
            emit resultReceived(imageId, text, cleanedImage, success, error);
      
            std::cout << "Received result for: " << imageId.toStdString() << std::endl;
        }
    });
    
    // Send all images
    int image_counter = 0;
    for (const QString& imagePath : imagePaths) {
        QFile file(imagePath);

        if (!file.open(QIODevice::ReadOnly)) {
            std::cerr << "Failed to open file: " << imagePath.toStdString() << std::endl;
            continue;
        }
        
        QByteArray imageData = file.readAll();
        file.close();
    
        ocr::ImageRequest request;
        request.set_image_id(QString("img_%1_%2")
            .arg(batchId)
            .arg(++image_counter)
            .toStdString());
        request.set_image_data(imageData.data(), imageData.size());
        request.set_batch_id(batchId.toStdString());
        
        if (!stream->Write(request)) {
            std::cerr << "Failed to write request for: " << imagePath.toStdString() << std::endl;
            break;
        }
        
        std::cout << "Sent image: " << request.image_id() << std::endl;
    }
    
    // Signal we're done writing
    stream->WritesDone();
    
    // Wait for reader thread
    if (reader_thread.joinable()) {
        reader_thread.join();
    }
    
    grpc::Status status = stream->Finish();
    if (!status.ok()) {
        emit connectionError(QString("gRPC error: %1").arg(QString::fromStdString(status.error_message())));
        std::cerr << "gRPC error: " << status.error_code() << ": " << status.error_message() << std::endl;
    }
}
