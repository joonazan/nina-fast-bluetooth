#pragma once
#include "protocol/ble_uuid.h"
#include "protocol/protocol.h"
#include "Arduino.h"

#define UUID_128(bytes...) (ble_uuid_any_t){ .u128 = { u: {type: BLE_UUID_TYPE_128}, value: { bytes } } }

class BLEClass;

namespace BLETypes {
  template <typename T>
  class Output {
    uint16_t _n;
    Output(uint16_t x) : _n(x) {}
    friend BLEClass;
  public:
    Output() {}
    void write(T obj) {
      Serial2.write((uint8_t*)&_n, 2);
      Serial2.write((uint8_t*)&obj, sizeof(T));
    }
  };

  class IInput {
    virtual void receive();
    virtual void setup();
    friend BLEClass;
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

    Input(ble_uuid_any_t id, void (*cb)(T)) : _cb(cb), _uuid(id) {};
    friend BLEClass;
  };
}

class BLEClass {
public:
  template <typename T>
  BLETypes::Output<T> add_output(ble_uuid_any_t id) {
    init_if_not_initialized();

    SetupMessage msg
      {
      type: BLE_CREATE_OUTPUT,
      uuid: id,
      length: sizeof(T),
      };
    Serial2.write((uint8_t*)&msg, sizeof(SetupMessage));

    return { _output_count++ };
  }

  template <typename T>
  BLETypes::Input<T> make_input(ble_uuid_any_t id, void (*cb)(T)) {
    _input_count++;
    return { id, cb };
  };

  void start(ble_uuid_any_t, BLETypes::IInput**);
  void poll();

private:
  bool _initialized = false;
  void init_if_not_initialized();

  uint16_t _output_count = 0;
  uint16_t _input_count = 0;
  BLETypes::IInput** _inputs;
};

extern BLEClass BLE;
