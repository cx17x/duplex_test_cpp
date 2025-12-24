#include "simulation.h"

SimulationRunner::SimulationRunner(DeviceA& deviceA, DeviceB& deviceB)
    : deviceA_(deviceA), deviceB_(deviceB), running_(false) {}

SimulationRunner::~SimulationRunner() {
    stop();
}

void SimulationRunner::start() {
    if (started_) {
        return;
    }
    running_ = true;
    threadA_ = std::thread([this]() { deviceA_.run(running_); });
    threadB_ = std::thread([this]() { deviceB_.run(running_); });
    started_ = true;
}

void SimulationRunner::stop() {
    if (!started_) {
        return;
    }
    running_ = false;
    if (threadA_.joinable()) {
        threadA_.join();
    }
    if (threadB_.joinable()) {
        threadB_.join();
    }
    started_ = false;
}

void SimulationRunner::runFor(std::chrono::milliseconds duration) {
    ensureStarted();
    std::this_thread::sleep_for(duration);
    stop();
}

void SimulationRunner::ensureStarted() {
    if (!started_) {
        start();
    }
}
