#include <FastBLE.h>

void cb(uint8_t) {
}

static const ble_uuid_any_t id = { .u128 = { u: {type: BLE_UUID_TYPE_128}, value: {0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59} } };

// This line MUST be before any lines creating Outputs.
BLE ble;

BLE::Output<uint64_t> out(id);

void setup() {
  BLE::Input<uint8_t> in(id, cb);
  BLE::IInput* inputs[] = { &in };

  ble.start(id, inputs);

  Serial.begin(115200);
  Serial.write("Started");
}

void loop() {
  ble.poll();
}