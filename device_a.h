#pragma once

#include <atomic>
#include <cstdint>
#include <random>
#include <vector>

#include "protocol.h"
#include "virtual_serial.h"

class DeviceA {
public:
    explicit DeviceA(VirtualSerial& serial);

    void run(std::atomic<bool>& running);
    void sendPidCommand(float kp, float ki, float kd);

private:
    void sendRandomCommand();
    void sendPwmCommand(uint16_t value);
    void requestStatus();
    void processResponses();
    void handleMessage(const std::vector<uint8_t>& message);

    VirtualSerial& serial_;
    protocol::Protocol protocol_;
    std::mt19937 rng_;
    std::uniform_int_distribution<int> pwmDistribution_;
    std::bernoulli_distribution commandSelector_;
};
