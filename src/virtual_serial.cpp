#include "virtual_serial.h"

void VirtualSerial::sendAtoB(const std::vector<uint8_t>& data) {
    {
        std::lock_guard<std::mutex> lock(mutexAtoB_);
        queueAtoB_.push_back(data);
    }
    cvAtoB_.notify_one();
}

void VirtualSerial::sendBtoA(const std::vector<uint8_t>& data) {
    {
        std::lock_guard<std::mutex> lock(mutexBtoA_);
        queueBtoA_.push_back(data);
    }
    cvBtoA_.notify_one();
}

bool VirtualSerial::readA(std::vector<uint8_t>& out) {
    return pop(queueBtoA_, mutexBtoA_, out);
}

bool VirtualSerial::readB(std::vector<uint8_t>& out) {
    return pop(queueAtoB_, mutexAtoB_, out);
}

bool VirtualSerial::waitReadA(std::vector<uint8_t>& out, std::chrono::milliseconds timeout) {
    return waitAndPop(queueBtoA_, mutexBtoA_, cvBtoA_, out, timeout);
}

bool VirtualSerial::waitReadB(std::vector<uint8_t>& out, std::chrono::milliseconds timeout) {
    return waitAndPop(queueAtoB_, mutexAtoB_, cvAtoB_, out, timeout);
}

bool VirtualSerial::waitAndPop(std::deque<std::vector<uint8_t>>& queue,
                               std::mutex& mutex,
                               std::condition_variable& cv,
                               std::vector<uint8_t>& out,
                               std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex);
    if (!cv.wait_for(lock, timeout, [&queue]() { return !queue.empty(); })) {
        return false;
    }
    out = queue.front();
    queue.pop_front();
    return true;
}

bool VirtualSerial::pop(std::deque<std::vector<uint8_t>>& queue,
                        std::mutex& mutex,
                        std::vector<uint8_t>& out) {
    std::lock_guard<std::mutex> lock(mutex);
    if (queue.empty()) {
        return false;
    }
    out = queue.front();
    queue.pop_front();
    return true;
}
