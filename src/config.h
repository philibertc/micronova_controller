// Compile-time configuration: pins, factory defaults and protocol constants.
// Everything marked "default" can be changed at runtime from the web interface;
// the values here are only used on first boot or after a factory reset.
#pragma once

#include <Arduino.h>

#define FIRMWARE_VERSION "2.0.0"

// ---------------------------------------------------------------------------
// Pins (Micronova Controller PCB, ESP8266 / Wemos D1 mini)
// ---------------------------------------------------------------------------
#define RESET_PIN D5 // Short to GND to erase the WiFi credentials and open the setup portal
#define RX_PIN D3
#define TX_PIN D4
#define ENABLE_RX D2 // Drives the optocoupler LED of the RX line (LOW = listening)

// ---------------------------------------------------------------------------
// Factory defaults
// ---------------------------------------------------------------------------
#define DEFAULT_HOSTNAME "micronova"
#define DEFAULT_MQTT_SERVER "" // Empty = MQTT disabled
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_MQTT_TOPIC "micronova"
#define DEFAULT_MQTT_USER ""
#define DEFAULT_MQTT_PASS ""
#define DEFAULT_HYDRO_MODE false
#define DEFAULT_POLL_INTERVAL_S 25
#define DEFAULT_WEB_USER "admin"
#define DEFAULT_WEB_PASS "micronova" // Also used as the OTA password. Empty = no authentication

// ---------------------------------------------------------------------------
// Network behaviour
// ---------------------------------------------------------------------------
#define WIFI_CONNECT_TIMEOUT_MS 30000UL // After this, the setup access point is opened (STA keeps retrying)
#define SETUP_AP_PREFIX "Micronova-Setup"
#define MQTT_RETRY_INTERVAL_MS 5000UL
#define RESET_PIN_HOLD_MS 1000UL // RESET_PIN must stay LOW this long to trigger a reset

// ---------------------------------------------------------------------------
// Micronova serial protocol
// ---------------------------------------------------------------------------
// Frames: read = [code, address], reply = [checksum, value]
//         write = [code, address, value, checksum], checksum = (code + address + value) & 0xFF
#define STOVE_BAUD 1200
#define STOVE_SERIAL_MODE SWSERIAL_8N2 // 8 data bits, no parity, 2 stop bits
#define STOVE_REPLY_DELAY_MS 80

#define READ_RAM 0x00
#define READ_EEPROM 0x20
#define WRITE_RAM 0x80
#define WRITE_EEPROM 0xA0

// Values written to the state register
#define STOVE_CMD_ON 0x01
#define STOVE_CMD_OFF 0x06
#define STOVE_CMD_FORCE_OFF 0x00
