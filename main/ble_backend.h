#pragma once
#include "protocol.h"

typedef void write_callback_fn(size_t, uint8_t*);

void BleBackend_init(const char*, const ble_uuid_t*, Characteristic*, size_t, Characteristic*, size_t, write_callback_fn*);

void change_char(size_t, uint8_t*);