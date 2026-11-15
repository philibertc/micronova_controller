// Stove tests that used to be separate sketches in tests/esp8266, now runnable from the web interface.
// Long tests (address scan, on/off cycle) are state machines advanced by diagLoop(), so the
// web server and MQTT keep running while they execute.
#pragma once

#include <Arduino.h>
#include "stove.h"

enum DiagTest : uint8_t
{
    DIAG_IDLE = 0,
    DIAG_SCAN,  // Reads every address in a range (test2bis_read_all_addresses)
    DIAG_ONOFF, // Turns the stove on, waits, turns it off (test1_on_off)
};

struct DiagStatus
{
    DiagTest running;
    DiagTest last; // Type of the last test started (results stay available after it ends)
    uint8_t memory;
    uint16_t start, end, current;
    uint16_t okCount, errorCount;
    unsigned long startedAt;
};

void diagBegin();
void diagLoop();
bool diagBusy();
const DiagStatus &diagStatus();

bool diagStartScan(uint8_t memory, uint8_t start, uint8_t end);
bool diagStartOnOff(uint16_t onSeconds);
void diagStop();

// Scan results: value of an address, or -1 if it could not be read (or was not scanned)
int16_t diagScanValue(uint8_t address);

// Short text log of what the tests did (oldest first)
uint8_t diagLogCount();
const String &diagLogLine(uint8_t index);
void diagLog(const String &line);
