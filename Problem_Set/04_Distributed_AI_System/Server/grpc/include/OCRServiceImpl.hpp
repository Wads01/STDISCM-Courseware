#pragma once

#include <grpcpp/grpcpp.h>
#include "ocr_service.grpc.pb.h"
#include "OCRPipeline.hpp"
#include <memory>
#include <thread>
#include <vector>

class OCRServiceImpl final : public ocr::OCRService::Service {
public:
    OCRServiceImpl(unsigned int num_workers = 0);
    ~OCRServiceImpl();

    grpc::Status ProcessImages(grpc::ServerContext* context, grpc::ServerReaderWriter<ocr::OCRResult, ocr::ImageRequest>* stream) override;

private:
    void workerThread();
    void processImage(const ocr::ImageRequest& request, ocr::OCRResult* result);
    
    std::vector<std::unique_ptr<OCRPipeline>> pipelines_;
    std::vector<std::thread> workers_;
    unsigned int num_workers_;
    std::atomic<bool> shutdown_;
};
