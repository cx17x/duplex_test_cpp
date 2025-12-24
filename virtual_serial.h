#pragma once

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <vector>
#include <chrono>

class VirtualSerial {
public:
    void sendAtoB(const std::vector<uint8_t>& data);
    void sendBtoA(const std::vector<uint8_t>& data);

    bool readA(std::vector<uint8_t>& out);
    bool readB(std::vector<uint8_t>& out);

    bool waitReadA(std::vector<uint8_t>& out, std::chrono::milliseconds timeout);
    bool waitReadB(std::vector<uint8_t>& out, std::chrono::milliseconds timeout);

private:
    bool waitAndPop(std::deque<std::vector<uint8_t>>& queue,
                    std::mutex& mutex,
                    std::condition_variable& cv,
                    std::vector<uint8_t>& out,
                    std::chrono::milliseconds timeout);

    bool pop(std::deque<std::vector<uint8_t>>& queue,
             std::mutex& mutex,
             std::vector<uint8_t>& out);

    std::deque<std::vector<uint8_t>> queueAtoB_;
    std::deque<std::vector<uint8_t>> queueBtoA_;
    std::mutex mutexAtoB_;
    std::mutex mutexBtoA_;
    std::condition_variable cvAtoB_;
    std::condition_variable cvBtoA_;
};
