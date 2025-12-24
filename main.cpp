#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "device_a.h"
#include "device_b.h"
#include "virtual_serial.h"

int main() {
    VirtualSerial channel;
    DeviceA deviceA(channel);
    DeviceB deviceB(channel);

    deviceA.sendPidCommand(1.0f, 0.1f, 0.01f);

    std::atomic<bool> running{true};

    std::thread threadA([&]() { deviceA.run(running); });
    std::thread threadB([&]() { deviceB.run(running); });

    std::this_thread::sleep_for(std::chrono::seconds(5));
    running = false;

    threadA.join();
    threadB.join();

    std::cout << "Simulation finished." << std::endl;
    return 0;
}
