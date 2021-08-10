# micronova_controller ![ESP8266](https://img.shields.io/badge/ESP-8266-000000.svg?longCache=true&style=flat&colorA=CCCC33) ![ESP32](https://img.shields.io/badge/ESP-32-000000.svg?longCache=true&style=flat&colorA=FF3500)
<a href="https://www.paypal.com/donate?hosted_button_id=Q48QBGGQTVGTG">
  <img src="https://raw.githubusercontent.com/stefan-niedermann/paypal-donate-button/master/paypal-donate-button.png" alt="Donate with PayPal" width=150 />
</a>

This Arduino program allows to control easily via MQTT a pellet stove equipped with a MicroNova mainboard.

## What you need
txt

## The circuit
![image](https://user-images.githubusercontent.com/57588282/128608590-39b0e0ff-b224-42bd-bb20-524a78f282f6.png)

`Enable_RX` is D2 (GPIO4) on ESP8266 or GPIO35 on ESP32.  
`RX` is D3 (GPIO0) on ESP8266 or GPIO32 on ESP32.  
`TX` is D4 (GPIO2) on ESP8266 or GPIO33 on ESP32.

The optocouplers are used to convert between 3.3V and 5V logic and also to protect the pellet stove and the ESP from each other in case of problem.
## Uploading firmware
### With esptool or NodeMCU PyFlasher
- Download the hex file corresponding to your board and your language in the [releases section](https://github.com/philibertc/micronova_controller/releases),
- Connect your ESP to your computer,
- Flash your ESP with that hex file (for esptool: `esptool write_flash *.hex`).
### With Arduino IDE
- Install ESP cards with the board manager (custom URLs: http://arduino.esp8266.com/stable/package_esp8266com_index.json,https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json),
- Install WifiManager and PubSubClient libraries,
- Copy the content of `micronova-controller.ino`,
- Connect your ESP to your computer and flash.

## Configure WiFi and the MQTT broker URL
- Connect to the `Pellet Stove Controller` WiFi network.
- Go to 192.168.4.1.
- Setup your WiFi credentials.
- Setup `mqtt_server` (the IP of your MQTT broker).
- Optionally adapt `mqtt_port` if you aren't using 1883.
- Optionally adapt `base_topic` if you have multiple stoves.
- Optionally setup `mqtt_user` and `mqtt_pass`.
- Optionally set `hydro_mode` to `1` if your pellet stove has this feature.

## MQTT details
First of all, if you use Home Assistant read [this paragraph](#home-assistant).
### The different topics
- The `mqtt_topic`, it's the common part between all the topics (default: `micronova`).
- The `ambtemp_topic`, this is the topic where you can read the ambient temperature (`mqtt_topic`**`/ambtemp`**).
- The `fumetemp_topic`, this is the topic on which you can read the temperature of the fumes (`mqtt_topic`**`/fumetemp`**).
- The `watertemp_topic`, this is the topic on which you can read the water temperature (only if you have a "idro" pellet stove) (`mqtt_topic`**`/watertemp`**).
- The `flame_topic`, this is the topic on which you can read the power of the flame (0, 1, 2, 3, 4, 5) (`mqtt_topic`**`/flamepower`**).
- The `state_topic`, this is the topic containing the detailed status of the stove (`mqtt_topic`**`/state`**).
- The `onoff_topic`, this is the topic informing if the stove is turned off or on (`mqtt_topic`**`/onoff`**).
- The `in_topic`, it is the topic allowing to control the stove (`mqtt_topic`**`/intopic`**).

### Controlling the stove
- To light the stove send `ON` to `in_topic`, __without retain flag__.
- To turn off the stove send `OFF` to `in_topic`, with or without retain flag.
- To change the flame power send `1`, `2`, `3`, `4`, or `5` to `in_topic`, without retain flag.

## Reset all settings
If you want to reset all the settings here is what you can do:
- send `reset` to `in_topic` or
- connect the `RESET_PIN` (D5 (GPIO14) on ESP8266 or GPIO25 on ESP32) to GND (this is not the RST pin of the ESP) or
- `esptool erase_flash` but this will also delete the program.
## Home Assistant
Here is what to add to `configuration.yaml`:
