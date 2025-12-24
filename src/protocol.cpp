#include "protocol.h"

#include <sstream>
#include <stdexcept>

namespace protocol {

namespace {

constexpr uint16_t kPolynomial = 0x1021;
constexpr uint16_t kInitValue = 0xFFFF;
constexpr uint16_t kXorOut = 0xFFFF;

uint8_t reflect8(uint8_t data) {
    uint8_t reflection = 0;
    for (int i = 0; i < 8; ++i) {
        if (data & 0x01) {
            reflection |= (1u << (7 - i));
        }
        data >>= 1;
    }
    return reflection;
}

uint16_t reflect16(uint16_t data) {
    uint16_t reflection = 0;
    for (int i = 0; i < 16; ++i) {
        if (data & 0x01) {
            reflection |= (1u << (15 - i));
        }
        data >>= 1;
    }
    return reflection;
}

}  // namespace

uint16_t Protocol::crc16X25(const std::vector<uint8_t>& data) const {
    uint16_t crc = kInitValue;
    for (uint8_t byte : data) {
        byte = reflect8(byte);
        crc ^= static_cast<uint16_t>(byte) << 8;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ kPolynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    crc = reflect16(crc) ^ kXorOut;
    return crc;
}

std::vector<uint8_t> Protocol::serialize(uint8_t commandId, const std::vector<uint8_t>& payload) const {
    if (payload.size() > 255) {
        throw std::runtime_error("Payload too large");
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(4 + payload.size() + 2);
    buffer.push_back(kHeader);
    buffer.push_back(kVersion);
    buffer.push_back(commandId);
    buffer.push_back(static_cast<uint8_t>(payload.size()));
    buffer.insert(buffer.end(), payload.begin(), payload.end());

    uint16_t crc = crc16X25(buffer);
    buffer.push_back(static_cast<uint8_t>(crc & 0xFF));
    buffer.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
    return buffer;
}

bool Protocol::deserialize(const std::vector<uint8_t>& data, Packet& packet, std::string& error) const {
    error.clear();
    if (data.size() < 6) {
        error = "Packet too short";
        return false;
    }

    if (data[0] != kHeader) {
        error = "Invalid header";
        return false;
    }

    if (data[1] != kVersion) {
        error = "Unsupported version";
        return false;
    }

    uint8_t payloadLen = data[3];
    size_t expectedSize = 4 + payloadLen + 2;
    if (data.size() != expectedSize) {
        error = "Length mismatch";
        return false;
    }

    uint16_t providedCrc = static_cast<uint16_t>(data[data.size() - 2]) |
                           (static_cast<uint16_t>(data[data.size() - 1]) << 8);
    std::vector<uint8_t> withoutCrc(data.begin(), data.end() - 2);
    uint16_t computed = crc16X25(withoutCrc);
    if (computed != providedCrc) {
        error = "CRC mismatch";
        return false;
    }

    packet.commandId = data[2];
    packet.payload.assign(data.begin() + 4, data.end() - 2);
    return true;
}

}  // namespace protocol
