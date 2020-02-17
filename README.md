# Fast Bluetooth on Arduino MKR Wifi 1010

The stock ArduinoBLE library does all the bluetooth logic on the main processor
and makes the ESP32 of the NINA-W102 just forward all communication to the
bluetooth unit.

BLE.poll() may wait as long as the connection interval. This wastes all the
processing power the ARM core has to offer.

One could make the existing implementation asynchronous. Instead, this project
aims to do as much of the bluetooth logic on the ESP32 as possible. The main
processor shall only tell the coprocessor when some characteristic should change.
