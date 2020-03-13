/*
 * Multiplication service
 * 
 * Creates a BLE Service that accepts has a 32-bit integer input and a 64-bit integer output.
 * The output is the input shifted left by 8 bits.
 */

#include <FastBLE.h>

void setup() {
}

BLE::Output<uint64_t> out;

/* 
 * Callback that is later attached to one input Characteristic of the Service 
 */
void cb(int32_t x) {
  out.write((int64_t)x << 8);
}

void loop() {
  /*
   * FastBLE object's constructors may only be called after the Arduino library has initialized itself.
   * That makes them inconvenient to initialize in setup, so I've hijacked loop instead.
   */
  BLE ble;
  out = BLE::Output<uint64_t>(UUID_128(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59));

  BLE::Input<int32_t> in(UUID_128(0x2a, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59), cb);
  BLE::IInput* inputs[] = { &in };

  ble.start(UUID_128(0x2c, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0xED, 0xFE), inputs);

  while(true) {
    ble.poll();
  }
}