#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <cstring>

namespace protocol {

constexpr uint8_t kHeader = 0xAA;
constexpr uint8_t kVersion = 0x01;

class Protocol;

enum class CommandId : uint8_t {
    SetPwm = 0x10,
    SetPid = 0x20,
    RequestStatus = 0x30
};

enum class ResponseId : uint8_t {
    AckPwm = 0x81,
    AckPid = 0x82,
    Status = 0x83
};

struct Packet {
    uint8_t commandId = 0;
    std::vector<uint8_t> payload;
};

class Protocol {
public:
    std::vector<uint8_t> serialize(uint8_t commandId, const std::vector<uint8_t>& payload) const;
    bool deserialize(const std::vector<uint8_t>& data, Packet& packet, std::string& error) const;

private:
    uint16_t crc16X25(const std::vector<uint8_t>& data) const;
};

template <typename T>
void appendPod(std::vector<uint8_t>& buffer, const T& value) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
}

template <typename T>
T readPod(const uint8_t* data) {
    T value{};
    std::memcpy(&value, data, sizeof(T));
    return value;
}

}  // namespace protocol
