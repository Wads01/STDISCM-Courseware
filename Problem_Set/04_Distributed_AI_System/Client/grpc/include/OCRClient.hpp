#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "ocr_service.grpc.pb.h"

class OCRClient : public QObject {
    Q_OBJECT

public:
    explicit OCRClient(const QString& server_address, QObject* parent = nullptr);
    ~OCRClient();

    void uploadImages(const QStringList& imagePaths, const QString& batchId);

signals:
    void resultReceived(const QString& imageId, const QString& text, 
    const QByteArray& cleanedImage, bool success, const QString& error);
    void connectionError(const QString& error);

private:
    void processImages(const QStringList& imagePaths, const QString& batchId);
    
    std::unique_ptr<ocr::OCRService::Stub> stub_;
    QString server_address_;
};
