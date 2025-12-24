#pragma once

#include <atomic>
#include <chrono>
#include <thread>

#include "device_a.h"
#include "device_b.h"

class SimulationRunner {
public:
    SimulationRunner(DeviceA& deviceA, DeviceB& deviceB);
    ~SimulationRunner();

    void start();
    void stop();
    void runFor(std::chrono::milliseconds duration);

private:
    void ensureStarted();

    DeviceA& deviceA_;
    DeviceB& deviceB_;
    std::atomic<bool> running_;
    std::thread threadA_;
    std::thread threadB_;
    bool started_ = false;
};
