#include "utils.hpp"
#include <cstdlib>
#include <cstddef>


std::string byte_to_hex(uint8_t byte)
{
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    return ss.str();
}

std::string bytes_to_hex_string(const std::vector<uint8_t> &bytes)
{
    std::stringstream ss;
    for (uint8_t byte : bytes)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    return ss.str();
}

std::vector<uint8_t> hex_string_to_bytes(const std::string &hex)
{
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2)
    {
        std::string byte_string = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(strtol(byte_string.c_str(), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

uint8_t calc_crc(const std::vector<uint8_t> &message_bytes)
{
    if (message_bytes.empty())
    {
        return false;
    }
    uint8_t crc = 0;
    for (const auto &byte : message_bytes)
    {
        crc += byte;
    }
    return crc & 0xFF;
}

bool verify_checksum(const std::vector<uint8_t> &message_bytes)
{
    if (message_bytes.empty())
    {
        return false;
    }
    uint8_t crc = 0;
    for (size_t i = 0; i < message_bytes.size() - 1; ++i)
    {
        crc += message_bytes[i];
    }
    return (crc & 0xFF) == message_bytes.back();
}
