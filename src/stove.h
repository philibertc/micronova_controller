// Micronova serial protocol: register access, metric polling and stove commands.
#pragma once

#include <Arduino.h>
#include "metrics.h"

enum ReadResult : uint8_t
{
    READ_OK = 0,
    READ_NO_REPLY,     // Nothing received
    READ_BAD_LENGTH,   // Received something, but not exactly 2 bytes
    READ_BAD_CHECKSUM, // 2 bytes received but checksum does not match the request
};

const char *readResultName(ReadResult r);

struct StoveData
{
    bool valid[METRIC_COUNT];
    uint8_t raw[METRIC_COUNT];
    float value[METRIC_COUNT];
    unsigned long lastUpdate[METRIC_COUNT]; // millis() of the last successful read
    int state;                              // -1 until the state has been read once
    unsigned long lastPoll;
    uint32_t readOk;
    uint32_t readErrors;
};

extern StoveData stoveData;

void stoveBegin();

// Low level access ---------------------------------------------------------
// code is READ_RAM / READ_EEPROM. rawReply receives the bytes received (up to 2), rawLen their count.
ReadResult stoveReadRegister(uint8_t code, uint8_t address, uint8_t &value,
                             uint8_t *rawReply = nullptr, uint8_t *rawLen = nullptr);
// code is WRITE_RAM / WRITE_EEPROM. The stove does not acknowledge writes.
void stoveWriteRegister(uint8_t code, uint8_t address, uint8_t value);

inline uint8_t readCodeFor(uint8_t memory) { return memory == MEM_EEPROM ? READ_EEPROM : READ_RAM; }
inline uint8_t writeCodeFor(uint8_t memory) { return memory == MEM_EEPROM ? WRITE_EEPROM : WRITE_RAM; }

// Metrics ------------------------------------------------------------------
bool metricActive(MetricId id); // Enabled and relevant (hydro metrics only in hydro mode)
bool stoveReadMetric(MetricId id);
void stovePollAll();
float metricRawToValue(MetricId id, uint8_t raw);
String formatMetric(MetricId id); // Text value as published on MQTT (state name for M_STATE)

// Polling schedule: pollDue() returns true every pollInterval seconds, or after requestPoll()
void requestPoll(unsigned long delayMs = 0);
bool pollDue();

// Commands -----------------------------------------------------------------
// turnOn/turnOff only send the command when it makes sense for the current state
// (same rules as the original firmware) unless force is true. They return true if a command was sent.
bool stoveTurnOn(bool force = false);
bool stoveTurnOff(bool force = false);
void stoveForceOff();
bool stoveSetTemperature(float celsius);
bool stoveSetPower(uint8_t power);
