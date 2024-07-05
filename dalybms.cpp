#include "dalybms.hpp"
#include "utils.hpp"
#include "error_codes.h"

const char *DalyBMS::TAG = "DalyBMS";

DalyBMS::DalyBMS()
    : nb_cells_(0),
      nb_temperature_sensors_(0),
      uart_port(UART_NUM_1)
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    ESP_LOGD(TAG, "Configuring UART with baud rate: %d", UART_BAUD);
    if (uart_param_config(uart_port, &uart_config) == ESP_FAIL)
    {
        ESP_LOGE(TAG, "Failed to configure UART parameters");
    }

    ESP_LOGD(TAG, "Setting UART pins (TX: %d, RX: %d, RTS: %d, CTS: %d)", UART_TXD, UART_RXD, UART_RTS, UART_CTS);
    if (uart_set_pin(uart_port, UART_TXD, UART_RXD, UART_RTS, UART_CTS) == ESP_FAIL)
    {
        ESP_LOGE(TAG, "Failed to set UART pins");
    }

    ESP_LOGD(TAG, "Installing UART driver with buffer size: %d", UART_BUFFER_SIZE * 2);
    if (uart_driver_install(uart_port, UART_BUFFER_SIZE * 2, 0, 0, NULL, 0) == ESP_FAIL)
    {
        ESP_LOGE(TAG, "Failed to install UART driver");
    }

    ESP_LOGI(TAG, "UART connected successfully");
}

DalyBMS::~DalyBMS()
{
    uart_driver_delete(uart_port);
}

std::vector<uint8_t> DalyBMS::format_message(const std::string &command, const std::string &extra)
{
    std::string message = "a5" + byte_to_hex(BMS_ADDRESS) + command + "08" + extra;
    message = message + std::string(24 - message.length(), '0');

    std::vector<uint8_t> message_bytes = hex_string_to_bytes(message);
    uint8_t crc = calc_crc(message_bytes);

    message_bytes.push_back(crc);
    return message_bytes;
}

std::vector<std::vector<uint8_t>> DalyBMS::read_request(const std::string &command, const std::string &extra, int max_responses, int timeout)
{
    std::vector<std::vector<uint8_t>> response_data;
    for (int x = 0; x < BMS_RETRIES; ++x)
    {
        response_data = read(command, extra, max_responses, timeout);
        if (response_data.empty())
        {
            ESP_LOGW(TAG, "No data read from serial port on attempt %d", x + 1);
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        else
        {
            break;
        }
    }
    return response_data;
}

std::vector<std::vector<uint8_t>> DalyBMS::read(const std::string &command, const std::string &extra, int max_responses, int timeout)
{
    uart_flush(uart_port);
    std::vector<uint8_t> message_bytes = format_message(command, extra);
    uart_write_bytes(uart_port, reinterpret_cast<const char *>(message_bytes.data()), message_bytes.size());

    vTaskDelay(pdMS_TO_TICKS(200));

    std::vector<std::vector<uint8_t>> response_data;
    int x = 0;
    uint8_t buffer[13];

    // Convert command to its byte representation for comparison
    uint8_t expected_data_id = std::stoi(command, nullptr, 16);

    while (x < max_responses)
    {
        int len = uart_read_bytes(uart_port, buffer, sizeof(buffer), timeout / portTICK_PERIOD_MS);
        if (len <= 0)
        {
            break;
        }

        ESP_LOGD(TAG, "Raw status response: %s", bytes_to_hex_string(std::vector<uint8_t>(buffer, buffer + len)).c_str());

        if (buffer[2] != expected_data_id)
        {
            ESP_LOGW(TAG, "Data ID mismatch: expected 0x%02x, got 0x%02x", expected_data_id, buffer[2]);
            continue;
        }

        if (!verify_checksum(std::vector<uint8_t>(buffer, buffer + len)))
        {
            ESP_LOGW(TAG, "Checksum verification failed for received message on attempt %d", x + 1);
            continue;
        }
        std::vector<uint8_t> message(buffer + 4, buffer + len - 1);
        response_data.push_back(message);
        x++;
    }

    return response_data;
}

template <typename T>
T DalyBMS::unpack_single(const std::vector<uint8_t> &data, size_t &offset)
{
    T value;
    const uint8_t *start = data.data() + offset;
    const uint8_t *end = start + sizeof(T);
    std::reverse_copy(start, end, reinterpret_cast<uint8_t *>(&value));
    offset += sizeof(T);
    return value;
}

template <typename... Args>
std::tuple<Args...> DalyBMS::unpack_response(const std::vector<uint8_t> &response_data)
{
    size_t offset = 0;
    return std::make_tuple(unpack_single<Args>(response_data, offset)...);
}

std::map<std::string, float> DalyBMS::get_soc()
{
    auto response_data = read_request("90");
    if (response_data.empty())
        return {};

    std::map<std::string, float> data;
    auto parts = unpack_response<uint16_t, uint16_t, uint16_t, uint16_t>(response_data[0]);
    data["total_voltage"] = static_cast<float>(std::get<0>(parts));
    data["current"] = static_cast<float>(std::get<2>(parts) - 30000);
    data["soc_percent"] = static_cast<float>(std::get<3>(parts));
    for (const auto &[key, value] : data)
    {
        ESP_LOGD(TAG, "%s: %.2f", key.c_str(), value);
    }

    return data;
}

std::map<std::string, float> DalyBMS::get_cell_voltage_range()
{
    auto response_data = read_request("91");
    if (response_data.empty())
        return {};

    std::map<std::string, float> data;
    auto parts = unpack_response<int16_t, uint8_t, int16_t, uint8_t>(response_data[0]);
    data["highest_voltage"] = std::get<0>(parts) / 1000.0;
    data["highest_cell"] = static_cast<float>(std::get<1>(parts));
    data["lowest_voltage"] = std::get<2>(parts) / 1000.0;
    data["lowest_cell"] = static_cast<float>(std::get<3>(parts));
    for (const auto &[key, value] : data)
    {
        ESP_LOGD(TAG, "%s: %.2f", key.c_str(), value);
    }
    return data;
}

std::map<std::string, float> DalyBMS::get_temperature_range()
{
    auto response_data = read_request("92");
    if (response_data.empty())
        return {};

    std::map<std::string, float> data;
    auto parts = unpack_response<int8_t, int8_t, int8_t, int8_t>(response_data[0]);
    data["highest_temperature"] = std::get<0>(parts) - 40;
    data["highest_sensor"] = static_cast<float>(std::get<1>(parts));
    data["lowest_temperature"] = std::get<2>(parts) - 40;
    data["lowest_sensor"] = static_cast<float>(std::get<3>(parts));
    for (const auto &[key, value] : data)
    {
        ESP_LOGD(TAG, "%s: %.2f", key.c_str(), value);
    }
    return data;
}

std::map<std::string, float> DalyBMS::get_mosfet_status()
{
    auto response_data = read_request("93");
    if (response_data.empty())
        return {};

    std::map<std::string, float> data;
    auto parts = unpack_response<int8_t, bool, bool, uint8_t, int32_t>(response_data[0]);
    data["bms_state"] = std::get<0>(parts);
    data["charging_mosfet"] = std::get<1>(parts);
    data["discharging_mosfet"] = std::get<2>(parts);
    data["bms_life"] = std::get<3>(parts);
    data["capacity_ah"] = std::get<4>(parts) / 1000.0;
    for (const auto &[key, value] : data)
    {
        ESP_LOGD(TAG, "%s: %.2f", key.c_str(), value);
    }

    /**
    std::string mode;
    if (std::get<0>(parts) == 0)
    {
        mode = "stationary";
    }
    else if (std::get<0>(parts) == 1)
    {
        mode = "charging";
    }
    else
    {
        mode = "discharging";
    }
     */
    return data;
}

std::map<std::string, int> DalyBMS::get_status()
{
    auto response_data = read_request("94");
    if (response_data.empty())
        return {};

    std::map<std::string, int> data;
    auto parts = unpack_response<uint8_t, uint8_t, uint8_t, uint8_t>(response_data[0]);
    data["cells"] = static_cast<int>(std::get<0>(parts));
    data["temperature_sensors"] = static_cast<int>(std::get<1>(parts));
    data["charger_running"] = static_cast<int>(std::get<2>(parts));
    data["load_running"] = static_cast<int>(std::get<3>(parts));
    for (const auto &entry : data)
    {
        ESP_LOGD(TAG, "Status field: %s, value: %d", entry.first.c_str(), entry.second);
    }

    nb_cells_ = static_cast<int>(std::get<0>(parts));
    nb_temperature_sensors_ = static_cast<int>(std::get<1>(parts));
    return data;
}

std::map<int, int> DalyBMS::get_cell_voltages()
{
    int cell_id = 1;
    int num_per_frame = 3;
    int max_responses = (nb_cells_ + num_per_frame - 1) / num_per_frame;

    auto response_data = read_request("95", "", max_responses);
    if (response_data.empty())
        return {};

    std::map<int, int> data;
    for (const auto &frame : response_data)
    {
        int frame_number = frame[0];
        if (frame_number == 0xFF)
            continue; // Invalid frame
        size_t offset = 1;
        for (int j = 0; j < num_per_frame; ++j)
        {
            if (offset + 2 >= frame.size())
                break;
            if (cell_id > nb_cells_)
                break;
            data[cell_id] = unpack_single<uint16_t>(frame, offset);
            cell_id = cell_id + 1;
        }
    }
    for (const auto &[id, voltage] : data)
    {
        ESP_LOGD(TAG, "Cell %d voltage: %d", id, voltage);
    }

    return data;
}

std::map<int, int> DalyBMS::get_temperatures()
{
    int sensor_id = 1;
    int num_per_frame = 7;
    int max_responses = (nb_temperature_sensors_ + num_per_frame - 1) / num_per_frame;

    auto response_data = read_request("96", "", max_responses);
    if (response_data.empty())
        return {};

    std::map<int, int> data;
    for (const auto &frame : response_data)
    {
        int frame_number = frame[0];
        if (frame_number == 0xFF)
            continue; // Invalid frame
        size_t offset = 1;
        for (int j = 0; j < num_per_frame; ++j)
        {
            if (offset + 1 >= frame.size())
                break;
            if (sensor_id > nb_temperature_sensors_)
                break;
            data[sensor_id] = unpack_single<int8_t>(frame, offset) - 40;
            sensor_id = sensor_id + 1;
        }
    }
    for (const auto &[id, temperature] : data)
    {
        ESP_LOGD(TAG, "Sensor %d temperature: %d", id, temperature);
    }

    return data;
}

std::map<int, bool> DalyBMS::get_balancing_status()
{
    auto response_data = read_request("97");
    if (response_data.empty())
        return {};

    std::map<int, bool> balancing_status;
    auto parts = unpack_response<uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t>(response_data[0]);

    uint64_t part0 = static_cast<uint64_t>(std::get<0>(parts));
    uint64_t part1 = static_cast<uint64_t>(std::get<1>(parts));
    uint64_t part2 = static_cast<uint64_t>(std::get<2>(parts));
    uint64_t part3 = static_cast<uint64_t>(std::get<3>(parts));
    uint64_t part4 = static_cast<uint64_t>(std::get<4>(parts));
    uint64_t part5 = static_cast<uint64_t>(std::get<5>(parts));

    uint64_t combined = (part0 << 40) |
                        (part1 << 32) |
                        (part2 << 24) |
                        (part3 << 16) |
                        (part4 << 8) |
                        part5;

    std::bitset<48> bits(combined);

    for (size_t i = 0; i < bits.size(); ++i)
    {
        balancing_status[i + 1] = bits.test(i);
    }

    for (const auto &[cell, status] : balancing_status)
    {
        ESP_LOGD(TAG, "Cell %d balancing: %s", cell, status ? "true" : "false");
    }

    return balancing_status;
}

std::vector<std::string> DalyBMS::get_errors()
{
    auto response_data = read_request("98");
    if (response_data.empty())
        return {};

    std::vector<std::string> data;
    for (size_t i = 0; i < response_data[0].size(); ++i)
    {
        std::bitset<8> bits(response_data[0][i]);
        for (size_t j = 0; j < bits.size(); ++j)
        {
            if (bits.test(j))
            {
                data.push_back(ERROR_CODES[i][j]);
            }
        }
    }

    for (const auto &message : data)
    {
        ESP_LOGD(TAG, "Error: %s", message.c_str());
    }

    return data;
}

void DalyBMS::set_charge_mosfet(bool on)
{
    std::string extra = on ? "01" : "00";
    auto response_data = read_request("da", extra);
}

void DalyBMS::set_discharge_mosfet(bool on)
{
    std::string extra = on ? "01" : "00";
    auto response_data = read_request("d9", extra);
}