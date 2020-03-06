#pragma once
#include "host/ble_uuid.h"

typedef struct {
  ble_uuid_any_t uuid;
  uint16_t length;
} Characteristic;

static const int BAUDRATE = 115200;

// After this, (n_outputs + n_inputs) Characteristics should be sent.
typedef struct {
  ble_uuid_any_t service_uuid;
  uint16_t n_outputs;
  uint16_t n_inputs;
} SetupHeader;

// Characteristic updates are sent as the index of the characteristic
// in the list of inputs/outputs followed by the data.
