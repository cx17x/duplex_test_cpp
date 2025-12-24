#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "device_a.h"
#include "device_b.h"
#include "virtual_serial.h"
#include "simulation.h"

int main() {
    VirtualSerial channel;
    DeviceA deviceA(channel);
    DeviceB deviceB(channel);

    deviceA.sendPidCommand(1.0f, 0.1f, 0.01f);

    SimulationRunner runner(deviceA, deviceB);
    runner.runFor(std::chrono::seconds(5));

    std::cout << "Simulation finished." << std::endl;
    return 0;
}
