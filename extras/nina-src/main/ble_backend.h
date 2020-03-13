#pragma once
#include "../../../src/protocol/protocol.h"

typedef void write_callback_fn(size_t, uint8_t*);

void BleBackend_add_output(const ble_uuid_any_t* uuid, uint16_t length);
void BleBackend_add_input(const ble_uuid_any_t* uuid, uint16_t length);
void BleBackend_start(const char*, const ble_uuid_t*);
