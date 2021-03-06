#pragma once
#include "ble_uuid.h"

static const int BAUDRATE = 1843200;

// Setup

enum SetupMessageType
  {
   BLE_CREATE_OUTPUT = 1,
   BLE_CREATE_INPUT = 2,
   BLE_START = 3,
  };

// A number of Create Output / Input messages can be sent
//
// Then a Start message should be sent with the uuid field set to the
// UUID that the Service containing the characteristics should have and
// the length field set to the length of the device's name.
//
// Then length bytes (the name) should be sent.

typedef struct {
  uint8_t type;
  ble_uuid_any_t uuid;
  uint16_t length;
} SetupMessage;

// Operation

// Characteristic updates are sent as the index of the characteristic
// in the list of inputs/outputs followed by the data.
