#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>

std::string byte_to_hex(uint8_t byte);
std::string bytes_to_hex_string(const std::vector<uint8_t> &bytes);
std::vector<uint8_t> hex_string_to_bytes(const std::string &hex);
uint8_t calc_crc(const std::vector<uint8_t> &message_bytes);
bool verify_checksum(const std::vector<uint8_t> &message_bytes);

#endif // UTILS_HPP
