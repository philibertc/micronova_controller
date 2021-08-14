# micronova_controller ![ESP8266](https://img.shields.io/badge/ESP-8266-000000.svg?longCache=true&style=flat&colorA=CCCC33) ![ESP32](https://img.shields.io/badge/ESP-32-000000.svg?longCache=true&style=flat&colorA=FF3500)
<a href="https://www.paypal.com/donate?hosted_button_id=Q48QBGGQTVGTG">
  <img src="https://raw.githubusercontent.com/stefan-niedermann/paypal-donate-button/master/paypal-donate-button.png" alt="Donate with PayPal" width=150 />
</a>

This Arduino program allows to control easily via MQTT a pellet stove equipped with a MicroNova mainboard.

## Working with
With all stoves equipped with MicroNova mainboard. Known brands:
- AMG Spa
- Anselmo Cola
- Boreal
- Bronpi
- Corisit
- EL.FIRE
- EOSS
- EvaCalor
- Extraflame
- Fontana Calore
- Fonte Flamme
- Galletti
- Globe-Fire
- Jolly Mec
- Karmek
- Klover
- Laminox
- LMX
- Lorflam
- MCZ (MCZ, Cadel, Red, FreePoint)
- Moretti Design
- Nordic Fire
- Stufe a pellet Italia
- Thermoflux
- TS

## Confirmed working with
- RED Loto
- MCZ Kaika/Face

## What you need
- An ESP8266 (I recommend the D1 Mini). (approx. 4€)
- 3 PC817. (less than 2€ for 20pcs)
- 3 510 Ohms resistors. (less than 2€ for 100pcs)
- That's all!

## The circuit
![image](https://user-images.githubusercontent.com/57588282/128608590-39b0e0ff-b224-42bd-bb20-524a78f282f6.png)

`Enable_RX` is D2 (GPIO4) on ESP8266 or GPIO35 on ESP32.  
`RX` is D3 (GPIO0) on ESP8266 or GPIO32 on ESP32.  
`TX` is D4 (GPIO2) on ESP8266 or GPIO33 on ESP32.

The optocouplers are used to convert between 3.3V and 5V logic and also to protect the pellet stove and the ESP from each other in case of problem.

## Stove's mainboard pinout
There is a 4 pin connector (CN13) with:
- 5V
- 20V
- Serial
- GND

## Uploading firmware
### With esptool or NodeMCU PyFlasher
- Download the hex file corresponding to your board and your language in the [releases section](https://github.com/philibertc/micronova_controller/releases),
- Connect your ESP to your computer,
- Flash your ESP with that hex file (for esptool: `esptool write_flash *.hex`).
### With Arduino IDE
- Install ESP cards with the board manager (custom URLs: `http://arduino.esp8266.com/stable/package_esp8266com_index.json,https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`),
- Install WifiManager and PubSubClient libraries,
- Copy the content of `micronova_controller.ino`,
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
- The `watertemp_topic`, this is the topic on which you can read the water temperature (only if you have a hydro pellet stove) (`mqtt_topic`**`/watertemp`**).
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

## Disclaimer
THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
