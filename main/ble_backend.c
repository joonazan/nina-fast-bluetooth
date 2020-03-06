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

struct Chr {
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
struct {
  struct Chr* output_chars;
  uint16_t* input_lengths;
  write_callback_fn* write_cb;
} static bb;

void BleBackend_init(const char* name,
                     const ble_uuid_t* svc_id,
                     Characteristic* outputs, size_t n_outputs,
                     Characteristic* inputs, size_t n_inputs,
                     write_callback_fn *cb) {

  bb.write_cb = cb;

  size_t n_chars = n_outputs + n_inputs;

  // This is an intentional memory leak
  // If I ever want to make a deinit function, I should keep a pointer to this.
  struct ble_gatt_chr_def* characteristics =
    (struct ble_gatt_chr_def*) malloc(sizeof(struct ble_gatt_chr_def) * (n_chars + 1));

  bb.output_chars = (struct Chr*) malloc(sizeof(struct Chr) * n_outputs);

  for (size_t i = 0; i < n_outputs; i++) {
    characteristics[i] = (struct ble_gatt_chr_def)
      {
       .uuid = &outputs[i].uuid,
       .access_cb = chr_read_cb,
       .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
       .val_handle = &bb.output_chars[i].handle,
       .arg = (void*)i,
      };

    bb.output_chars[i].value = (uint8_t*) malloc(outputs[i].length);
    bb.output_chars[i].length = outputs[i].length;
  }

  bb.input_lengths = (uint16_t*) malloc(2 * n_inputs);

  for (size_t i = 0; i < n_inputs; i++) {
    characteristics[n_outputs + i] = (struct ble_gatt_chr_def)
      {
       .uuid = &inputs[i].uuid,
       .access_cb = chr_write_cb,
       .flags = BLE_GATT_CHR_F_WRITE,
       .arg = (void*)i,
      };

    bb.input_lengths[i] = inputs[i].length;
  }

  characteristics[n_chars] = (struct ble_gatt_chr_def) {0};

  struct ble_gatt_svc_def *svcs = (struct ble_gatt_svc_def*) malloc(2 * sizeof(struct ble_gatt_svc_def));

  svcs[0] = (struct ble_gatt_svc_def)
    {
     .type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = svc_id,
     .characteristics = characteristics,
    };

  svcs[1] = (struct ble_gatt_svc_def) { 0 };

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
}

void change_char(size_t i, uint8_t* value) {
  memcpy(bb.output_chars[i].value, value, bb.output_chars[i].length);
  ble_gatts_chr_updated(bb.output_chars[i].handle);
}

static int
chr_read_cb(uint16_t conn_handle, uint16_t attr_handle,
            struct ble_gatt_access_ctxt *ctxt,
            void *arg)
{
  assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

  size_t i = (size_t) arg;
  int rc = os_mbuf_append(ctxt->om, bb.output_chars[i].value, bb.output_chars[i].length);
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

  bb.write_cb(i, value);
  free(value);

  return 0;
}
