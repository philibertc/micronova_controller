# micronova_controller ![ESP8266](https://img.shields.io/badge/ESP-8266-000000.svg?longCache=true&style=flat&colorA=CC101F) ![ESP32](https://img.shields.io/badge/ESP-32-000000.svg?longCache=true&style=flat&colorA=CC101F)

This Arduino program allows to control easily via MQTT a pellet stove equipped with a MicroNova mainboard.

## The circuit

## Uploading firmware
### With esptool or NodeMCU PyFlasher
- Download the hex file corresponding to your board in the [releases section](https://github.com/philibertc/micronova_controller/releases),
- Connect your ESP to your computer,
- Flash your ESP with that hex file (for esptool: `esptool write-flash *.hex`).
### With Arduino IDE
- Install ESP cards with the board manager (custom URLs: http://arduino.esp8266.com/stable/package_esp8266com_index.json,https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json),
- Install WifiManager and PubSubClient libraries,
- Copy the content of `micronova-controller.ino`,
- Connect your ESP to your computer and flash!

## Configure WiFi and the MQTT broker URL
- Connect to the `Stove Controller` WiFi network,
- 
