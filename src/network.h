// WiFi station / setup access point, captive portal DNS, mDNS, OTA and the RESET_PIN.
#pragma once

#include <Arduino.h>

enum NetMode : uint8_t
{
    NET_SETUP,      // No credentials: access point only
    NET_CONNECTING, // Station connecting (fallback AP opens after WIFI_CONNECT_TIMEOUT_MS)
    NET_CONNECTED,
};

// firstBoot: no valid settings were found (fresh install or upgrade from 1.x)
void netBegin(bool firstBoot);
void netLoop();

NetMode netMode();
bool netApActive();
String netApSsid();
bool netStaConnected();

// Restarts the ESP after delayMs (lets the HTTP response go out first)
void netScheduleRestart(unsigned long delayMs = 1000);
