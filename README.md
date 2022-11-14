# Fast Bluetooth on Arduino MKR Wifi 1010

The stock ArduinoBLE library does all the bluetooth logic on the main processor
and makes the ESP32 of the NINA-W102 just forward all communication to the
bluetooth unit.

BLE.poll() may wait as long as the connection interval. This wastes all the
processing power the ARM core has to offer.

One could make the existing implementation asynchronous. Instead, this project
aims to do as much of the bluetooth logic on the ESP32 as possible. The main
processor shall only tell the coprocessor when some characteristic should change.

This is an Arduino library, but to use it, you need to first flash the NINA
coprocessor with my software, as most of the bluetooth code runs on the NINA.

## Flashing on Linux

- Upload SerialNINAPassthrough onto your board (found in File -> Examples -> FastBLE)
- Navigate to extras/nina-src
- Open a shell with ESP-IDF

    - Install [Nix](https://nixos.org/nix/)
    - run `nix --experimental-features 'nix-command flakes' develop github:mirrexagon/nixpkgs-esp-dev#esp32-idf`

    Alternatively, you can install ESP-IDF manually but I don't recommend it.

- Run `make flash`

Now code using the library should work on your board. If you later decide to
use the stock Wifi or bluetooth libraries, you need to flash the stock firmware
via the WiFiNINA Firmware Updater.

## Developing the library

You can open a serial monitor into the NINA with `make monitor`. It even shows
stack traces!
