#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

namespace sim {

    class Worker;

    class Sensor {
    public:
        using TemperatureArray = std::vector<std::atomic<float>>;

        Sensor(size_t sensorIndex, std::shared_ptr<TemperatureArray> sharedTemps, std::vector<size_t> neighborIndices);
        ~Sensor();

        Sensor(const Sensor&) = delete;
        Sensor& operator=(const Sensor&) = delete;

        void start();
        void stop();

        void setSleepDuration(std::chrono::milliseconds sleepMs);

        bool isRunning() const;

        float getCurrentTemperature() const;
        size_t getIndex() const noexcept { return sensorIndex_; }
        size_t getNeighborCount() const noexcept { return neighborIndices_.size(); }

    private:
        void calculateAndUpdate();

        size_t sensorIndex_;
        std::shared_ptr<TemperatureArray> sharedTemps_;
        std::vector<size_t> neighborIndices_;
        std::unique_ptr<Worker> worker_;
    };

}