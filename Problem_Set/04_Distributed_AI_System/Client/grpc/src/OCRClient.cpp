#include "OCRClient.hpp"

#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>

OCRClient::OCRClient(const QString& server_address, QObject* parent, size_t max_in_flight) : QObject(parent)
    , server_address_(server_address)
    , in_flight_slots_(max_in_flight)
    , max_in_flight_(max_in_flight)
    , shutdown_(false)
{
    auto channel = grpc::CreateChannel(server_address.toStdString(), grpc::InsecureChannelCredentials());
    stub_ = ocr::OCRService::NewStub(channel);

    std::cout << "OCR Client initialized for server: " << server_address.toStdString() << " (max in-flight: " << max_in_flight << ")" << std::endl;
}

OCRClient::~OCRClient()
{
    shutdown_.store(true);
}

void OCRClient::uploadImages(const QStringList& imagePaths, const QString& batchId)
{
    // Process in a separate thread to avoid blocking Qt event loop
    std::thread([this, imagePaths, batchId]() { processImages(imagePaths, batchId); }).detach();
}

void OCRClient::processImages(const QStringList& imagePaths, const QString& batchId)
{
    grpc::ClientContext context;
    auto stream = stub_->ProcessImages(&context);

    if (!stream) {
        emit connectionError("Failed to create gRPC stream");
        return;
    }

    std::atomic<int> results_received{0};
    std::atomic<bool> writer_done{false};
    std::atomic<size_t> current_in_flight{0};

    std::thread reader_thread([this, &stream, &results_received, &writer_done, &current_in_flight, imagePaths]() {
        ocr::OCRResult result;
        while (stream->Read(&result)) {
            QString imageId = QString::fromStdString(result.image_id());
            QString text = QString::fromStdString(result.extracted_text());
            bool success = result.success();
            QString error = QString::fromStdString(result.error_message());

            // Convert cleaned image bytes to QByteArray
            QByteArray cleanedImage;
            if (success && result.cleaned_image_data().size() > 0)
                cleanedImage = QByteArray(result.cleaned_image_data().data(), result.cleaned_image_data().size());

            // Emit queue depth for monitoring
            if (result.queue_depth() > 0)
                emit queueDepthUpdated(result.queue_depth());

            emit resultReceived(imageId, text, cleanedImage, success, error);

            // Release an in-flight slot
            in_flight_slots_.release();
            current_in_flight.fetch_sub(1);
            results_received++;

            std::cout << "Received result for: " << imageId.toStdString()
                      << " (results: " << results_received.load() << "/" << imagePaths.size() << ")"
                      << std::endl;
        }

        std::cout << "[Reader] Finished receiving results" << std::endl;
    });

    // Send all images with flow control
    int image_counter = 0;
    for (const QString& imagePath : imagePaths) {
        if (shutdown_.load()) {
            std::cout << "Client shutting down, stopping image upload" << std::endl;
            break;
        }

        // Wait for an available in-flight slot (client-side backpressure)
        in_flight_slots_.acquire();
        current_in_flight.fetch_add(1);

        QFile file(imagePath);

        if (!file.open(QIODevice::ReadOnly)) {
            std::cerr << "Failed to open file: " << imagePath.toStdString() << std::endl;
            in_flight_slots_.release();
            current_in_flight.fetch_sub(1);
            continue;
        }

        QByteArray imageData = file.readAll();
        file.close();

        ocr::ImageRequest request;
        request.set_image_id(QString("img_%1_%2").arg(batchId).arg(++image_counter).toStdString());
        request.set_image_data(imageData.data(), imageData.size());
        request.set_batch_id(batchId.toStdString());

        if (!stream->Write(request)) {
            std::cerr << "Failed to write request for: " << imagePath.toStdString() << std::endl;
            in_flight_slots_.release();
            current_in_flight.fetch_sub(1);
            break;
        }

        std::cout << "Sent image: " << request.image_id()
                  << " (in-flight: " << current_in_flight.load() << "/" << max_in_flight_ << ")"
                  << std::endl;
    }

    writer_done.store(true);

    stream->WritesDone();

    std::cout << "[Writer] Finished sending images, waiting for remaining results..." << std::endl;

    // Wait for reader thread
    if (reader_thread.joinable())
        reader_thread.join();

    grpc::Status status = stream->Finish();
    if (!status.ok()) {
        emit connectionError(QString("gRPC error: %1").arg(QString::fromStdString(status.error_message())));
        std::cerr << "gRPC error: " << status.error_code() << ": " << status.error_message() << std::endl;
    }

    std::cout << "[Client] Processing complete" << std::endl;
}
