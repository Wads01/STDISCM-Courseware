#include "Sensor.hpp"
#include "Worker.hpp"

#include <chrono>

namespace sim {

    Sensor::Sensor(size_t sensorIndex, std::shared_ptr<TemperatureArray> sharedTemps, std::vector<size_t> neighborIndices)
        : sensorIndex_(sensorIndex), sharedTemps_(std::move(sharedTemps)), neighborIndices_(std::move(neighborIndices)) {

        auto task = [this]() { calculateAndUpdate(); };
        worker_ = std::make_unique<Worker>(std::move(task));
    }

    Sensor::~Sensor() {
        stop();
    }

    void Sensor::start() {
        if (worker_)
            worker_->start();
    }

    void Sensor::stop() {
        if (worker_)
            worker_->stop();
    }

    void Sensor::setSleepDuration(std::chrono::milliseconds sleepMs) {
        if (worker_)
            worker_->setSleepDuration(sleepMs);
    }

    bool Sensor::isRunning() const {
        return worker_ && worker_->isRunning();
    }

    float Sensor::getCurrentTemperature() const {
        if (!sharedTemps_ || sensorIndex_ >= sharedTemps_->size())
            return 0.0f;

        return (*sharedTemps_)[sensorIndex_].load(std::memory_order_relaxed);
    }

    void Sensor::calculateAndUpdate() {
        if (!sharedTemps_ || sensorIndex_ >= sharedTemps_->size()) 
            return;

        if (neighborIndices_.empty())
            return;

        double sum = 0.0;
        size_t validNeighbors = 0;

        for (size_t neighborIndex : neighborIndices_) {
            // Bounds check
            if (neighborIndex < sharedTemps_->size()) {
                float neighborTemp = (*sharedTemps_)[neighborIndex].load(std::memory_order_relaxed);
                sum += static_cast<double>(neighborTemp);
                ++validNeighbors;
            }
        }

        // Calculate average and update
        if (validNeighbors > 0) {
            float average = static_cast<float>(sum / static_cast<double>(validNeighbors));
            (*sharedTemps_)[sensorIndex_].store(average, std::memory_order_relaxed);
        }
    }

}