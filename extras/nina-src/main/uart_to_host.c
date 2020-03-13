#include "driver/uart.h"
#include "../../../src/protocol/protocol.h"
#include "uart_to_host.h"

void init_uart() {
  uart_config_t uart_config =
    {
     .baud_rate = BAUDRATE,
     .data_bits = UART_DATA_8_BITS,
     .parity    = UART_PARITY_DISABLE,
     .stop_bits = UART_STOP_BITS_1,
     .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

  const int BUF_SIZE = 1024;

  uart_param_config(UART_NUM_1, &uart_config);
  uart_set_pin(UART_NUM_1, 23, 12, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
}

void read_from_uart(void* out, size_t len) {
  uint8_t *data = (uint8_t *) out;
  uint8_t *end = data + len;
  uint8_t *cursor = data;

  while (cursor != end) {
    int len = uart_read_bytes(UART_NUM_1, cursor, end - cursor, 100);
    if (len != -1) {
      cursor += len;
    }
  }
}

void write_to_uart(void* data, size_t len) {
  uart_write_bytes(UART_NUM_1, data, len);
}
