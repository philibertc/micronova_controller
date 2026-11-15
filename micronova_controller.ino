// Micronova Controller firmware 2.x
// Controls a pellet stove equipped with a Micronova mainboard over MQTT and a built-in web interface.
//
// Project layout:
//   src/config.h        Pins, factory defaults and protocol constants
//   src/metrics.h       Values read from the stove and their default addresses
//   src/settings.*      Persistent settings (WiFi, MQTT, addresses...) in the emulated EEPROM
//   src/stove.*         Micronova serial protocol, polling and commands
//   src/network.*       WiFi, setup access point / captive portal, mDNS, OTA, RESET_PIN
//   src/mqtt.*          MQTT and Home Assistant discovery
//   src/diagnostics.*   Address scan, on/off test (formerly tests/esp8266)
//   src/web_server.*    HTTP server and JSON API
//   src/web_ui.h        Web interface (HTML/CSS/JS)
//
// First boot: connect to the "Micronova-Setup-XXXX" WiFi network and open http://192.168.4.1/
// Hold RESET_PIN (D5) to GND for 1 s to erase the WiFi credentials and reopen the setup network.

#include "src/config.h"
#include "src/settings.h"
#include "src/stove.h"
#include "src/network.h"
#include "src/mqtt.h"
#include "src/diagnostics.h"
#include "src/web_server.h"

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("Micronova Controller " FIRMWARE_VERSION));

    stoveBegin();
    bool validSettings = settingsLoad();
    diagBegin();
    netBegin(!validSettings);
    mqttBegin();
    webBegin();
    requestPoll(2000);
}

void loop()
{
    netLoop();
    webLoop();
    mqttLoop();
    diagLoop();

    if (!diagBusy() && pollDue()) // Tests use the serial line, so the periodic polling waits for them
    {
        stovePollAll();
        mqttPublishMetrics();
    }

    if (mqttDeepSleepRequested()) // Does not work without hardware modification (a cable must be connected between RST and D0)
    {
        Serial.println(F("Deep Sleep"));
        ESP.deepSleepInstant(300e6);
    }
}
