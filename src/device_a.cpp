#include "device_a.h"

#include <chrono>
#include <iostream>
#include <thread>

DeviceA::DeviceA(VirtualSerial& serial)
    : serial_(serial),
      rng_(std::random_device{}()),
      pwmDistribution_(0, 2000),
      commandSelector_(0.5) {}

void DeviceA::run(std::atomic<bool>& running) {
    using namespace std::chrono_literals;
    while (running.load()) {
        sendRandomCommand();
        processResponses();
        std::this_thread::sleep_for(100ms);
    }
}

void DeviceA::sendRandomCommand() {
    if (commandSelector_(rng_)) {
        sendPwmCommand(static_cast<uint16_t>(pwmDistribution_(rng_)));
    } else {
        requestStatus();
    }
}

void DeviceA::sendPwmCommand(uint16_t value) {
    std::vector<uint8_t> payload;
    payload.reserve(sizeof(uint16_t));
    protocol::appendPod(payload, value);
    auto packet = protocol_.serialize(static_cast<uint8_t>(protocol::CommandId::SetPwm), payload);
    serial_.sendAtoB(packet);
    std::cout << "[A] Sent PWM command: value=" << value << std::endl;
}

void DeviceA::sendPidCommand(float kp, float ki, float kd) {
    std::vector<uint8_t> payload;
    payload.reserve(sizeof(float) * 3);
    protocol::appendPod(payload, kp);
    protocol::appendPod(payload, ki);
    protocol::appendPod(payload, kd);
    auto packet = protocol_.serialize(static_cast<uint8_t>(protocol::CommandId::SetPid), payload);
    serial_.sendAtoB(packet);
    std::cout << "[A] Sent PID command: kp=" << kp << " ki=" << ki << " kd=" << kd << std::endl;
}

void DeviceA::requestStatus() {
    std::vector<uint8_t> payload;
    auto packet = protocol_.serialize(static_cast<uint8_t>(protocol::CommandId::RequestStatus), payload);
    serial_.sendAtoB(packet);
    std::cout << "[A] Requested status" << std::endl;
}

void DeviceA::processResponses() {
    std::vector<uint8_t> message;
    if (serial_.waitReadA(message, std::chrono::milliseconds(10))) {
        handleMessage(message);
    }
    while (serial_.readA(message)) {
        handleMessage(message);
    }
}

void DeviceA::handleMessage(const std::vector<uint8_t>& message) {
    protocol::Packet packet;
    std::string error;
    if (!protocol_.deserialize(message, packet, error)) {
        std::cout << "[A] Failed to parse response: " << error << std::endl;
        return;
    }

    switch (packet.commandId) {
        case static_cast<uint8_t>(protocol::ResponseId::AckPwm): {
            if (packet.payload.size() != sizeof(uint16_t)) {
                std::cout << "[A] Invalid PWM ack payload" << std::endl;
                return;
            }
            uint16_t pwm = protocol::readPod<uint16_t>(packet.payload.data());
            std::cout << "[A] Received ack: pwm=" << pwm << std::endl;
            break;
        }
        case static_cast<uint8_t>(protocol::ResponseId::AckPid): {
            if (packet.payload.size() != sizeof(float) * 3) {
                std::cout << "[A] Invalid PID ack payload" << std::endl;
                return;
            }
            float kp = protocol::readPod<float>(packet.payload.data());
            float ki = protocol::readPod<float>(packet.payload.data() + sizeof(float));
            float kd = protocol::readPod<float>(packet.payload.data() + 2 * sizeof(float));
            std::cout << "[A] Received PID ack: kp=" << kp << " ki=" << ki << " kd=" << kd << std::endl;
            break;
        }
        case static_cast<uint8_t>(protocol::ResponseId::Status): {
            if (packet.payload.size() != 2) {
                std::cout << "[A] Invalid status payload" << std::endl;
                return;
            }
            uint8_t battery = packet.payload[0];
            uint8_t temperature = packet.payload[1];
            std::cout << "[A] Received status: battery=" << static_cast<int>(battery)
                      << " temp=" << static_cast<int>(temperature) << std::endl;
            break;
        }
        default:
            std::cout << "[A] Unknown response id: 0x" << std::hex << static_cast<int>(packet.commandId)
                      << std::dec << std::endl;
            break;
    }
}
