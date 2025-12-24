#include "device_b.h"

#include <chrono>
#include <iostream>
#include <thread>

DeviceB::DeviceB(VirtualSerial& serial)
    : serial_(serial),
      rng_(std::random_device{}()),
      batteryDistribution_(0, 99),
      temperatureDistribution_(20, 49) {}

void DeviceB::run(std::atomic<bool>& running) {
    using namespace std::chrono_literals;
    while (running.load()) {
        processCommands();
        std::this_thread::sleep_for(10ms);
    }
}

void DeviceB::processCommands() {
    std::vector<uint8_t> message;
    if (serial_.waitReadB(message, std::chrono::milliseconds(10))) {
        handleCommand(message);
    }
    while (serial_.readB(message)) {
        handleCommand(message);
    }
}

void DeviceB::handleCommand(const std::vector<uint8_t>& message) {
    protocol::Packet packet;
    std::string error;
    if (!protocol_.deserialize(message, packet, error)) {
        std::cout << "[B] Failed to parse command: " << error << std::endl;
        return;
    }

    switch (packet.commandId) {
        case static_cast<uint8_t>(protocol::CommandId::SetPwm):
            handleSetPwm(packet);
            break;
        case static_cast<uint8_t>(protocol::CommandId::SetPid):
            handleSetPid(packet);
            break;
        case static_cast<uint8_t>(protocol::CommandId::RequestStatus):
            handleRequestStatus();
            break;
        default:
            std::cout << "[B] Unknown command id: 0x" << std::hex << static_cast<int>(packet.commandId)
                      << std::dec << std::endl;
            break;
    }
}

void DeviceB::handleSetPwm(const protocol::Packet& packet) {
    if (packet.payload.size() != sizeof(uint16_t)) {
        std::cout << "[B] Invalid PWM payload size" << std::endl;
        return;
    }
    uint16_t pwm = protocol::readPod<uint16_t>(packet.payload.data());
    std::cout << "[B] Applying PWM value: " << pwm << std::endl;
    sendAckPwm(pwm);
}

void DeviceB::handleSetPid(const protocol::Packet& packet) {
    if (packet.payload.size() != sizeof(float) * 3) {
        std::cout << "[B] Invalid PID payload size" << std::endl;
        return;
    }
    float kp = protocol::readPod<float>(packet.payload.data());
    float ki = protocol::readPod<float>(packet.payload.data() + sizeof(float));
    float kd = protocol::readPod<float>(packet.payload.data() + 2 * sizeof(float));
    std::cout << "[B] Applying PID: kp=" << kp << " ki=" << ki << " kd=" << kd << std::endl;
    sendAckPid(kp, ki, kd);
}

void DeviceB::handleRequestStatus() {
    auto battery = static_cast<uint8_t>(batteryDistribution_(rng_));
    auto temperature = static_cast<uint8_t>(temperatureDistribution_(rng_));
    std::cout << "[B] Reporting status: battery=" << static_cast<int>(battery)
              << " temp=" << static_cast<int>(temperature) << std::endl;
    sendStatus(battery, temperature);
}

void DeviceB::sendAckPwm(uint16_t value) {
    std::vector<uint8_t> payload;
    payload.reserve(sizeof(uint16_t));
    protocol::appendPod(payload, value);
    auto packet = protocol_.serialize(static_cast<uint8_t>(protocol::ResponseId::AckPwm), payload);
    serial_.sendBtoA(packet);
}

void DeviceB::sendAckPid(float kp, float ki, float kd) {
    std::vector<uint8_t> payload;
    payload.reserve(sizeof(float) * 3);
    protocol::appendPod(payload, kp);
    protocol::appendPod(payload, ki);
    protocol::appendPod(payload, kd);
    auto packet = protocol_.serialize(static_cast<uint8_t>(protocol::ResponseId::AckPid), payload);
    serial_.sendBtoA(packet);
}

void DeviceB::sendStatus(uint8_t battery, uint8_t temperature) {
    std::vector<uint8_t> payload{battery, temperature};
    auto packet = protocol_.serialize(static_cast<uint8_t>(protocol::ResponseId::Status), payload);
    serial_.sendBtoA(packet);
}
