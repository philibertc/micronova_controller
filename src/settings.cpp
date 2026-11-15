#include "settings.h"

#include <EEPROM.h>

#define SETTINGS_MAGIC 0x4D4E4F56UL // "MNOV"
#define SETTINGS_VERSION 1
#define EEPROM_SIZE 1024

static_assert(sizeof(Settings) <= EEPROM_SIZE, "Settings do not fit in the emulated EEPROM");

Settings settings;

static uint32_t computeCrc(const Settings &s)
{
    // CRC-32 over everything but the crc field itself
    const uint8_t *data = reinterpret_cast<const uint8_t *>(&s);
    size_t len = offsetof(Settings, crc);
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++)
            crc = (crc >> 1) ^ (0xEDB88320UL & (0 - (crc & 1)));
    }
    return ~crc;
}

void copyString(char *dst, size_t size, const String &src)
{
    strncpy(dst, src.c_str(), size - 1);
    dst[size - 1] = '\0';
}

void settingsResetMetric(MetricId id)
{
    settings.metrics[id].address = METRIC_INFO[id].defaultAddress;
    settings.metrics[id].memory = METRIC_INFO[id].defaultMemory;
    settings.metrics[id].divisor = METRIC_INFO[id].defaultDivisor;
    settings.metrics[id].enabled = 1;
}

void settingsSetDefaults()
{
    memset(&settings, 0, sizeof(settings));
    settings.magic = SETTINGS_MAGIC;
    settings.version = SETTINGS_VERSION;
    COPY_SETTING(hostname, DEFAULT_HOSTNAME);
    COPY_SETTING(mqttServer, DEFAULT_MQTT_SERVER);
    settings.mqttPort = DEFAULT_MQTT_PORT;
    COPY_SETTING(mqttUser, DEFAULT_MQTT_USER);
    COPY_SETTING(mqttPass, DEFAULT_MQTT_PASS);
    COPY_SETTING(mqttTopic, DEFAULT_MQTT_TOPIC);
    COPY_SETTING(webUser, DEFAULT_WEB_USER);
    COPY_SETTING(webPass, DEFAULT_WEB_PASS);
    settings.hydroMode = DEFAULT_HYDRO_MODE;
    settings.pollInterval = DEFAULT_POLL_INTERVAL_S;
    for (uint8_t i = 0; i < METRIC_COUNT; i++)
        settingsResetMetric((MetricId)i);
}

bool settingsLoad()
{
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(0, settings);
    bool valid = settings.magic == SETTINGS_MAGIC && settings.version == SETTINGS_VERSION &&
                 settings.crc == computeCrc(settings);
    if (!valid)
    {
        Serial.println(F("[settings] No valid settings found, using defaults"));
        settingsSetDefaults();
        return false;
    }
    // Sanity checks
    if (settings.pollInterval < 5)
        settings.pollInterval = 5;
    for (uint8_t i = 0; i < METRIC_COUNT; i++)
        if (settings.metrics[i].divisor == 0)
            settings.metrics[i].divisor = 1;
    return true;
}

bool settingsSave()
{
    settings.magic = SETTINGS_MAGIC;
    settings.version = SETTINGS_VERSION;
    settings.crc = computeCrc(settings);
    EEPROM.put(0, settings);
    bool ok = EEPROM.commit();
    Serial.printf("[settings] Saved (%s)\n", ok ? "ok" : "FAILED");
    return ok;
}

void settingsClearWifi()
{
    memset(settings.wifiSsid, 0, sizeof(settings.wifiSsid));
    memset(settings.wifiPass, 0, sizeof(settings.wifiPass));
    settingsSave();
}

bool settingsHasWifi()
{
    return settings.wifiSsid[0] != '\0';
}
