#pragma once
#include "protocol/ble_uuid.h"
#include "protocol/protocol.h"
#include "Arduino.h"

#define UUID_128(bytes...) (ble_uuid_any_t){ .u128 = { u: {type: BLE_UUID_TYPE_128}, value: { bytes } } }

extern uint16_t output_count;
extern uint16_t input_count;

class BLE {
public:
  template <typename T>
  class Output {
    uint16_t _n;
  public:
    Output() {}

    Output(ble_uuid_any_t id) {
      _n = output_count++;

      SetupMessage msg
        {
        type: BLE_CREATE_OUTPUT,
        uuid: id,
        length: sizeof(T),
        };
      Serial2.write((uint8_t*)&msg, sizeof(SetupMessage));
    };

    void write(T obj) {
      Serial2.write((uint8_t*)&_n, 2);
      Serial2.write((uint8_t*)&obj, sizeof(T));
    }
  };

  class IInput {
    virtual void receive();
    virtual void setup();
    friend BLE;
  };

  template <typename T>
  class Input : public IInput {
    void (*_cb)(T);
    ble_uuid_any_t _uuid;

    virtual void setup() {
      SetupMessage msg
        {
        type: BLE_CREATE_INPUT,
        uuid: _uuid,
        length: sizeof(T),
        };
      Serial2.write((uint8_t*)&msg, sizeof(SetupMessage));
    }

    virtual void receive() {
      T x;
      Serial2.readBytes((uint8_t*)&x, sizeof(T));
      _cb(x);
    }
  public:
    Input(ble_uuid_any_t id, void (*cb)(T)) : _cb(cb), _uuid(id) { input_count++; };
  };

  BLE();
  void start(ble_uuid_any_t, IInput**);
  void poll();

private:
  IInput** _inputs;
};
