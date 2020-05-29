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

void BLEClass::start(ble_uuid_any_t svc_id, const char *name, BLETypes::IInput** inputs) {
  init_if_not_initialized();

  _inputs = inputs;

  for (uint16_t i = 0; i < _input_count; i++) {
    inputs[i]->setup();
  }

  uint16_t name_length = 0;
  while(name[name_length++] != '\0');

  SetupMessage msg
    {
    type: BLE_START,
    uuid: svc_id,
    length: name_length,
    };
  Serial2.write((uint8_t*)&msg, sizeof(SetupMessage));
  Serial2.write((uint8_t*)name, name_length);
}

void BLEClass::poll() {
  if (Serial2.available() >= 2) {
    uint16_t input_no;
    Serial2.readBytes((uint8_t*)&input_no, 2);
    _inputs[input_no]->receive();
  }
}
