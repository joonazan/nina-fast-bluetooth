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

void app_main(void)
{
  output_chars[0].uuid = ui.u;
  BleBackend_init("testname", &gatt_svr_svc_sec_test_uuid.u, NULL, 0, output_chars, 1, write_callback);
}
