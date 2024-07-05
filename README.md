# ESP_DalyBMS

This project is an implementation to interface a Daly BMS with an ESP32 microcontroller using the ESP-IDF framework. The project is written in C++ and uses UART to communicate with the BMS.

## Project Structure

- **CMakeLists.txt**: CMake configuration file to compile the project.
- **Kconfig.projbuild**: Project configuration for ESP-IDF.
- **dalybms.cpp**: Main file containing the implementation of functions to communicate with the Daly BMS.
- **utils.cpp**: Utility functions used in the project.
- **include/**: Directory containing the header files (.hpp and .h) for the project.

## Configuration

Configuration is handled through the `Kconfig.projbuild` file, which can be modified using the `idf.py menuconfig` command.

## Usage

To use this project, follow these steps:

1. **Install dependencies**: Ensure the ESP-IDF framework is properly installed and configured.
2. **Add Component**: Copy the `ESP_DalyBMS` directory to the `components` directory of your ESP-IDF project.
3. **Configuration**: Use `idf.py menuconfig` to modify "Component - DalyBMS UART Settings" as needed.
4. **Build**: Use `idf.py build` to compile your project.
5. **Flash**: Use `idf.py flash` to flash the firmware to your ESP32.
6. **Monitor**: Use `idf.py monitor` to monitor the serial messages sent by the ESP32.

## Class Usage

The core functionality is encapsulated in the `DalyBMS` class, which provides methods to interact with the Daly BMS. Below is a simple example demonstrating how to use the `DalyBMS` class:

### Example

```cpp
#include "dalybms.hpp"
#include <iostream>

void example_usage() {
    DalyBMS bms_handler;

    auto soc = bms_handler.get_soc();
    auto mosfet_status = bms_handler.get_mosfet_status();
    auto status = bms_handler.get_status();
    auto cell_voltages = bms_handler.get_cell_voltages();
    auto temperatures = bms_handler.get_temperatures();
    auto errors = bms_handler.get_errors();

    // Example of using the retrieved data
    std::cout << "Total Voltage: " << soc["total_voltage"] << std::endl;
    std::cout << "Current: " << soc["current"] << std::endl;
    std::cout << "SOC Percentage: " << soc["soc_percent"] << std::endl;
    // Add more handling as needed
}
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contributions

Contributions are welcome! Please submit a pull request or open an issue to discuss any changes you wish to make.

## Authors

- [Lionel ORCIL](https://github.com/ioio2995)
