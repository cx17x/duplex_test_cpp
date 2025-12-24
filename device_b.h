#pragma once

#include <atomic>
#include <random>
#include <vector>

#include "protocol.h"
#include "virtual_serial.h"

class DeviceB {
public:
    explicit DeviceB(VirtualSerial& serial);

    void run(std::atomic<bool>& running);

private:
    void processCommands();
    void handleCommand(const std::vector<uint8_t>& message);
    void handleSetPwm(const protocol::Packet& packet);
    void handleSetPid(const protocol::Packet& packet);
    void handleRequestStatus();

    void sendAckPwm(uint16_t value);
    void sendAckPid(float kp, float ki, float kd);
    void sendStatus(uint8_t battery, uint8_t temperature);

    VirtualSerial& serial_;
    protocol::Protocol protocol_;
    std::mt19937 rng_;
    std::uniform_int_distribution<int> batteryDistribution_;
    std::uniform_int_distribution<int> temperatureDistribution_;
};
