#include "FastBLE.h"

uint16_t output_count = 0;
uint16_t input_count = 0;

BLE::BLE() {
  // reset Nina
  pinMode(NINA_RESETN, OUTPUT);
  digitalWrite(NINA_RESETN, HIGH);
  delay(100);
  digitalWrite(NINA_RESETN, LOW);
  delay(750);

  Serial2.begin(BAUDRATE);
}

void BLE::start(ble_uuid_any_t svc_id, IInput** inputs) {
  _inputs = inputs;

  for (uint16_t i = 0; i < input_count; i++) {
    inputs[i]->setup();
  }

  SetupMessage msg
    {
    type: BLE_START,
    uuid: svc_id,
    };
  Serial2.write((uint8_t*)&msg, sizeof(SetupMessage));
}

void BLE::poll() {
  uint16_t input_no;
  Serial2.readBytes((uint8_t*)&input_no, 2);
  _inputs[input_no]->receive();
}
