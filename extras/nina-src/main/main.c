#include "host/ble_hs.h"
#include "ble_backend.h"
#include "uart_to_host.h"

void app_main(void)
{
  init_uart();

  while (1) {
    SetupMessage msg;
    read_from_uart(&msg, sizeof(SetupMessage));

    ble_uuid_any_t* id = malloc(sizeof(ble_uuid_any_t));
    *id = msg.uuid;

    if (msg.type == BLE_CREATE_OUTPUT) {
      BleBackend_add_output(id, msg.length);
    } else if (msg.type == BLE_CREATE_INPUT) {
      BleBackend_add_input(id, msg.length);
    } else if (msg.type == BLE_START) {
      const uint16_t namelen = msg.length;
      char *name = malloc(namelen + 1);
      read_from_uart(name, namelen);
      name[namelen] = '\0';

      BleBackend_start(name, &id->u);
      break;
    }
  }
}
