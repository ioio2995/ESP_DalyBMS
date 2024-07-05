#ifndef UART_CONFIG_HPP
#define UART_CONFIG_HPP

#include "driver/uart.h"
#include "esp_log.h"

#define UART_TXD (CONFIG_BMS_TXD)
#define UART_RXD (CONFIG_BMS_RXD)
#define UART_RTS (CONFIG_BMS_RTS)
#define UART_CTS (CONFIG_BMS_CTS)
#define UART_BAUD 9600
#define UART_BUFFER_SIZE (1024)

#define BMS_ADDRESS (CONFIG_BMS_ADDRESS)
#define BMS_RETRIES (CONFIG_BMS_RETRIES)

#endif // UART_CONFIG_HPP
