# micronova_controller ![ESP8266](https://img.shields.io/badge/ESP-8266-000000.svg?longCache=true&style=flat&colorA=CC101F) ![ESP32](https://img.shields.io/badge/ESP-32-000000.svg?longCache=true&style=flat&colorA=CC101F)

This Arduino program allows to control easily via MQTT a pellet stove equipped with a MicroNova mainboard.

## The circuit
On breadboard:
![image](https://user-images.githubusercontent.com/57588282/127031692-c1dd1c95-987d-42d3-bf3d-23146b8f2eec.png)
Electrical schema:
![image](https://user-images.githubusercontent.com/57588282/127032442-d4bb1e1b-df1b-4e90-8297-b0b436b67163.png)

## Uploading firmware
### With esptool or NodeMCU PyFlasher
- Download the hex file corresponding to your board in the [releases section](https://github.com/philibertc/micronova_controller/releases),
- Connect your ESP to your computer,
- Flash your ESP with that hex file (for esptool: `esptool write_flash *.hex`).
### With Arduino IDE
- Install ESP cards with the board manager (custom URLs: http://arduino.esp8266.com/stable/package_esp8266com_index.json,https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json),
- Install WifiManager and PubSubClient libraries,
- Copy the content of `micronova-controller.ino`,
- Connect your ESP to your computer and flash.

## Configure WiFi and the MQTT broker URL
- Connect to the `Stove Controller` WiFi network,
- Setup your WiFi credentials,
- Setup MQTT base_url (the IP of your MQTT broker),
- Eventually configure the base_topic if you make several times this project.

## Home Assistant
Here is what to add to `configuration.yaml`:
```yaml
sensor:
  - platform: mqtt
    name: "Ambient temperature" #You can put what you want here.
    state_topic: "micronova/ambtemp" #base_topic/ambtemp
    qos: 0
    unit_of_measurement: "ºC"
    icon: mdi:thermometer #You can put what you want here.
  - platform: mqtt
    name: "Flue Gas Temperature" #You can put what you want here.
    state_topic: "micronova/fumetemp" #base_topic/fumetemp
    qos: 0
    unit_of_measurement: "°C"
    icon: mdi:thermometer #You can put what you want here.
```
```yaml
switch:
  - platform: mqtt
    name: "Stove" #You can put what you want here.
    state_topic: "micronova/onoff" #base_topic/onoff
    command_topic: "micronova/intopic" #base_topic/intopic
    payload_on: "ON"
    payload_off: "OFF"
    state_on: "ON"
    state_off: "OFF"
    optimistic: false
    qos: 0
    icon: mdi:fire #You can put what you want here.
```
```yaml
select:
  - platform: mqtt
    name: "Stove state" #You can put what you want here.
    state_topic: "micronova/state" #base_topic/state
    command_topic: "none" #We don't want to use that select entity to control the stove.
    options:
      - "Off"
      - "Starting"
      - "Pellet loading"
      - "Ignition"
      - "Work"
      - "Brazier cleaning"
      - "Final cleaning"
      - "Standby"
      - "Pellet missing"
      - "Ignition failure"
      - "Alarm"
    qos: 0
    icon: mdi:fire-alert #You can put what you want here.
```
