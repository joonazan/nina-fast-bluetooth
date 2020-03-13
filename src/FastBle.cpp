#include "FastBLE.h"

BLEClass BLE;

void BLEClass::init_if_not_initialized() {
  if (_initialized) {
    return;
  }
  _initialized = true;

  // reset Nina
  pinMode(NINA_RESETN, OUTPUT);
  digitalWrite(NINA_RESETN, HIGH);
  delay(100);
  digitalWrite(NINA_RESETN, LOW);
  delay(750);

  Serial2.begin(BAUDRATE);
  while(!Serial2);
}

void BLEClass::start(ble_uuid_any_t svc_id, BLETypes::IInput** inputs) {
  init_if_not_initialized();

  _inputs = inputs;

  for (uint16_t i = 0; i < _input_count; i++) {
    inputs[i]->setup();
  }

  SetupMessage msg
    {
    type: BLE_START,
    uuid: svc_id,
    };
  Serial2.write((uint8_t*)&msg, sizeof(SetupMessage));
}

void BLEClass::poll() {
  if (Serial2.available() >= 2) {
    uint16_t input_no;
    Serial2.readBytes((uint8_t*)&input_no, 2);
    _inputs[input_no]->receive();
  }
}
