#ifndef DALYBMS_HPP
#define DALYBMS_HPP

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <bitset>

#include "uart_config.hpp"

class DalyBMS {
public:
    DalyBMS();
    ~DalyBMS();
    
    std::map<std::string, float> get_soc();
    std::map<std::string, float> get_cell_voltage_range();
    std::map<std::string, float> get_temperature_range();
    std::map<std::string, float> get_mosfet_status();
    std::map<std::string, int> get_status();
    std::map<int, int> get_cell_voltages();
    std::map<int, int> get_temperatures();
    std::map<int, bool> get_balancing_status();
    std::vector<std::string> get_errors();
    void set_charge_mosfet(bool on);
    void set_discharge_mosfet(bool on);

private:
    std::vector<uint8_t> format_message(const std::string &command, const std::string &extra);
    std::vector<std::vector<uint8_t>> read_request(const std::string &command, const std::string &extra = "", int max_responses = 1, int wait = 200, int timeout = 100);
    std::vector<std::vector<uint8_t>> read(const std::string &command, const std::string &extra, int max_responses, int wait, int timeout);

    template<typename T> T unpack_single(const std::vector<uint8_t> &data, size_t &offset);
    template<typename... Args> std::tuple<Args...> unpack_response(const std::vector<uint8_t> &response_data);

    static const char *TAG;

    int nb_cells_;
    int nb_temperature_sensors_;
    uart_port_t uart_port;
};

#endif // DALYBMS_HELPER_HPP