// Persistent settings, stored in the emulated EEPROM (no filesystem partition required).
#pragma once

#include <Arduino.h>
#include "metrics.h"

struct Settings
{
    uint32_t magic;
    uint16_t version;

    char wifiSsid[33];
    char wifiPass[65];
    char hostname[33];

    char mqttServer[65]; // Empty = MQTT disabled
    uint16_t mqttPort;
    char mqttUser[33];
    char mqttPass[65];
    char mqttTopic[33];

    char webUser[33];
    char webPass[65]; // Empty = no authentication

    uint8_t hydroMode;
    uint16_t pollInterval; // Seconds between two stove polls

    MetricConfig metrics[METRIC_COUNT];

    uint32_t crc;
};

extern Settings settings;

// Loads the settings from flash, falling back to the factory defaults if they are missing or corrupted.
// Returns true if valid settings were found.
bool settingsLoad();
bool settingsSave();

void settingsSetDefaults();
void settingsResetMetric(MetricId id);
void settingsClearWifi(); // Erases only the WiFi credentials (RESET_PIN / "reset" command)

bool settingsHasWifi();

// Copies a String into a fixed size char array, always null terminated
void copyString(char *dst, size_t size, const String &src);
#define COPY_SETTING(field, value) copyString(settings.field, sizeof(settings.field), (value))
