#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include <semaphore>
#include <atomic>
#include <grpcpp/grpcpp.h>
#include "ocr_service.grpc.pb.h"

class OCRClient : public QObject {
    Q_OBJECT

public:
    explicit OCRClient(const QString& server_address, QObject* parent = nullptr, size_t max_in_flight = 50);
    ~OCRClient();

    void uploadImages(const QStringList& imagePaths, const QString& batchId);

signals:
    void resultReceived(const QString& imageId, const QString& text, 
    const QByteArray& cleanedImage, bool success, const QString& error);
    void connectionError(const QString& error);
    void queueDepthUpdated(int depth);

private:
    void processImages(const QStringList& imagePaths, const QString& batchId);
    
    std::unique_ptr<ocr::OCRService::Stub> stub_;
    QString server_address_;
    
    std::counting_semaphore<1024> in_flight_slots_;
    size_t max_in_flight_;
    std::atomic<bool> shutdown_;
};
