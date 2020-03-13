#include <assert.h>
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "esp_nimble_hci.h"
#include "ble_backend.h"
#include "ble_connection.h"
#include "uart_to_host.h"

struct OutputInfo {
  uint8_t *value;
  uint16_t length;
  uint16_t handle;
};

static ble_gatt_access_fn chr_read_cb;
static ble_gatt_access_fn chr_write_cb;

static void ble_task(void *param)
{
  ESP_LOGI("fast_BLE", "BLE Host Task Started");
  /* This function will return only when nimble_port_stop() is executed */
  nimble_port_run();

  nimble_port_freertos_deinit();
}

// You cannot have multiple BleBackends at the same time because of this
// global but I think you can't have multiple bluetooths anyway.
struct BleBackend {
  size_t input_count, output_count;

  struct ble_gatt_chr_def* characteristics;
  ble_uuid_any_t* uuids;
  struct OutputInfo* output_infos;
  uint16_t* input_lengths;

  write_callback_fn* write_cb;
};

static struct BleBackend bb = (struct BleBackend)
  {
   .input_count = 0,
   .output_count = 0,

   .characteristics = NULL,
   .uuids = NULL,
   .output_infos = NULL,
   .input_lengths = NULL,
  };

void BleBackend_add_output(const ble_uuid_any_t* uuid, uint16_t length) {
  size_t chr_count = bb.input_count + bb.output_count;
  bb.characteristics = realloc(bb.characteristics, (chr_count + 1) * sizeof(struct ble_gatt_chr_def));
  bb.output_infos = realloc(bb.output_infos, (bb.output_count + 1) * sizeof(struct OutputInfo));

  bb.output_infos[bb.output_count].value = (uint8_t*) malloc(length);
  bb.output_infos[bb.output_count].length = length;

  bb.characteristics[chr_count] = (struct ble_gatt_chr_def)
    {
     .uuid = &uuid->u,
     .access_cb = chr_read_cb,
     .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,

     // This is a relative address that gets offset in BleBackend_start
     // We cannot take an absolute address into output_infos, as it may get reallocated.
     .val_handle = (uint16_t*)((uint8_t*)&bb.output_infos[bb.output_count].handle - (uint8_t*)bb.output_infos),

     .arg = (void*)bb.output_count,
    };

  bb.output_count++;
}

void BleBackend_add_input(const ble_uuid_any_t* uuid, uint16_t length) {
  size_t chr_count = bb.input_count + bb.output_count;
  bb.characteristics = realloc(bb.characteristics, (chr_count + 1) * sizeof(struct ble_gatt_chr_def));
  bb.input_lengths = realloc(bb.input_lengths, (bb.input_count + 1) * sizeof(uint16_t));

  bb.input_lengths[bb.input_count] = length;

  bb.characteristics[chr_count] = (struct ble_gatt_chr_def)
    {
     .uuid = &uuid->u,
     .access_cb = chr_write_cb,
     .flags = BLE_GATT_CHR_F_WRITE,
     .arg = (void*)bb.input_count,
    };

  bb.input_count++;
}

void BleBackend_start(const char* name,
                     const ble_uuid_t* svc_id) {
  size_t chr_count = bb.input_count + bb.output_count;

  // add zero terminator
  bb.characteristics = realloc(bb.characteristics, (chr_count + 1) * sizeof(struct ble_gatt_chr_def));
  bb.characteristics[chr_count] = (struct ble_gatt_chr_def) {0};

  // convert relative addresses to absolute ones (see BleBackend_add_output)
  for (size_t i = 0; i < chr_count; i++) {
    if (bb.characteristics[i].val_handle != NULL) {
      bb.characteristics[i].val_handle = (uint16_t*)((uint8_t*)bb.output_infos + (size_t)bb.characteristics[i].val_handle);
    }
  }

  static struct ble_gatt_svc_def svcs[2];
  svcs[0] = (struct ble_gatt_svc_def)
    {
     .uuid = svc_id,
     .type = BLE_GATT_SVC_TYPE_PRIMARY,
     .characteristics = bb.characteristics,
    };
  svcs[1] = (struct ble_gatt_svc_def) {0};

  int rc;

  /* Initialize NVS — it is used to store PHY calibration data */
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());

  nimble_port_init();
  /* Initialize the NimBLE host configuration. */
  ble_hs_cfg.reset_cb = fast_ble_on_reset;
  ble_hs_cfg.sync_cb = fast_ble_on_sync;

  ble_svc_gap_init();
  ble_svc_gatt_init();

  rc = ble_gatts_count_cfg(svcs);
  assert(rc == 0);

  rc = ble_gatts_add_svcs(svcs);
  assert(rc == 0);

  rc = ble_svc_gap_device_name_set(name);
  assert(rc == 0);

  nimble_port_freertos_init(ble_task);

  // listen for output changes
  while (1) {
    uint16_t n;
    read_from_uart(&n, sizeof(uint16_t));

    read_from_uart(bb.output_infos[n].value, bb.output_infos[n].length);
    ble_gatts_chr_updated(bb.output_infos[n].handle);
  }
}

static int
chr_read_cb(uint16_t conn_handle, uint16_t attr_handle,
            struct ble_gatt_access_ctxt *ctxt,
            void *arg)
{
  assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

  size_t i = (size_t) arg;
  int rc = os_mbuf_append(ctxt->om, bb.output_infos[i].value, bb.output_infos[i].length);
  return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int
chr_write_cb(uint16_t conn_handle, uint16_t attr_handle,
            struct ble_gatt_access_ctxt *ctxt,
            void *arg)
{
  assert(ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR);

  size_t i = (size_t) arg;
  uint16_t expected_length = bb.input_lengths[i];
  uint8_t* value = (uint8_t*) malloc(expected_length);

  uint16_t actual_length;
  int err = ble_hs_mbuf_to_flat(ctxt->om, value, expected_length, &actual_length);
  if (err == BLE_HS_EMSGSIZE || actual_length != expected_length) {
    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
  } else if (err != 0) {
    return BLE_ATT_ERR_UNLIKELY;
  }

  uint16_t n = i;
  write_to_uart(&n, 2);
  write_to_uart(value, expected_length);

  free(value);

  return 0;
}
