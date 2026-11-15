// Description of the values read from the stove and their factory addresses.
#pragma once

#include <Arduino.h>
#include "config.h"

enum MetricId : uint8_t
{
    M_STATE = 0,
    M_AMB_TEMP,
    M_TEMP_SET,
    M_POWER_SET,
    M_FUMES_TEMP,
    M_FLAME_POWER,
    M_WATER_TEMP,
    M_WATER_PRES,
    METRIC_COUNT
};

enum MemoryType : uint8_t
{
    MEM_RAM = 0,
    MEM_EEPROM = 1
};

// Static description of a metric (never changes at runtime)
struct MetricInfo
{
    const char *key;   // JSON key / MQTT sub-topic
    const char *label; // Human readable name
    const char *unit;
    uint8_t defaultAddress;
    MemoryType defaultMemory;
    uint8_t defaultDivisor; // value = raw / divisor
    bool hydroOnly;         // Only polled when hydro mode is enabled
};

// Runtime (user editable) location of a metric
struct MetricConfig
{
    uint8_t address;
    uint8_t memory; // MemoryType
    uint8_t divisor;
    uint8_t enabled;
};

// Index = MetricId
static const MetricInfo METRIC_INFO[METRIC_COUNT] = {
    {"state", "Stove state", "", 0x21, MEM_RAM, 1, false},
    {"ambtemp", "Room temperature", "°C", 0x01, MEM_RAM, 2, false},
    {"tempset", "Temperature setpoint", "°C", 0x7D, MEM_EEPROM, 2, false},
    {"stovepower", "Power setpoint", "", 0x7F, MEM_EEPROM, 1, false},
    {"fumetemp", "Fumes temperature", "°C", 0x3E, MEM_RAM, 1, false},
    {"flamepower", "Flame power", "%", 0x34, MEM_RAM, 1, false},
    {"watertemp", "Water temperature", "°C", 0x03, MEM_RAM, 1, true},
    {"waterpres", "Water pressure", "bar", 0x3C, MEM_RAM, 10, true},
};

// 0 - Off, 1 - Starting, 2 - Pellet loading, 3 - Ignition, 4 - Working, 5 - Brazier cleaning,
// 6 - Final cleaning, 7 - Standby, 8 - Pellet missing alarm, 9 - Ignition failure alarm, 10 - Alarm
static const char *const STOVE_STATE_NAMES[] = {
    "Off", "Starting", "Pellet loading", "Ignition", "Working", "Brazier cleaning",
    "Final cleaning", "Standby", "Pellet missing", "Ignition failure", "Alarm"};
#define STOVE_STATE_COUNT (sizeof(STOVE_STATE_NAMES) / sizeof(STOVE_STATE_NAMES[0]))

inline const char *stoveStateName(int state)
{
    if (state < 0 || state >= (int)STOVE_STATE_COUNT)
        return "Unknown";
    return STOVE_STATE_NAMES[state];
}

// "ON" for states where the stove is burning (or about to), matching the original firmware
inline bool stoveStateIsOn(int state)
{
    return state >= 1 && state <= 5;
}
