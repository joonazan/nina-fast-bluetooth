/*
 * Multiplication service
 * 
 * Creates a BLE Service that accepts has a 32-bit integer input and a 64-bit integer output.
 * The output is the input shifted left by 8 bits.
 */

#include <FastBLE.h>

void setup() {}

BLETypes::Output<uint64_t> out;

// Input characteristics accept writes with a length corresponding to the callback's argument
// In this case four bytes
void cb(int32_t x) {
  out.write((int64_t)x << 8);
}

constexpr auto INPUT_UUID = UUID_128(0x2a, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);
constexpr auto OUTPUT_UUID = UUID_128(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);
constexpr auto SERVICE_UUID = UUID_128(0x2c, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0xED, 0xFE);

void loop() {
  // Outputs and inputs may only be added after the Arduino libraries have started up i.e. not in global scope
  out = BLE.add_output<uint64_t>(OUTPUT_UUID);

  auto in = BLE.make_input(INPUT_UUID, cb);

  // I do initialization in loop because this array must live as long as the program
  BLETypes::IInput* inputs[] = { &in };

  BLE.start(SERVICE_UUID, "multiplication service", inputs);

  while(true) {
    BLE.poll();
  }
}
