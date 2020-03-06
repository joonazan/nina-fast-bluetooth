#include "host/ble_hs.h"
#include "ble_backend.h"

void write_callback(size_t n, uint8_t* value) {
  MODLOG_DFLT(INFO, "got data %d\n", value[0]);
}

/* 59462f12-9543-9999-12c8-58b459a2712d */
static const ble_uuid128_t gatt_svr_svc_sec_test_uuid =
  BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12,
                   0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

static const ble_uuid128_t ui =
  BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12,
                   0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x52);

Characteristic output_chars[] =
  {
   {
    .length = 4,
   }
  };

#include "driver/uart.h"

static void init_uart() {
  uart_config_t uart_config =
    {
     .baud_rate = 115200,
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

void app_main(void)
{
  init_uart();

  output_chars[0].uuid.u128 = ui;
  BleBackend_init("testname", &gatt_svr_svc_sec_test_uuid.u, output_chars, 1, NULL, 0, write_callback);

  uint8_t *data = (uint8_t *) malloc(4);
  uint8_t *end = data + 4;
  uint8_t *cursor = data;

  while (1) {
    while (cursor != end) {
      int len = uart_read_bytes(UART_NUM_1, cursor, end - cursor, 100);
      if (len != -1) {
        cursor += len;
      }
    }
    change_char(0, data);
    cursor = data;
  }
}
