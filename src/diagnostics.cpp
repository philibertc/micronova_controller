#include "diagnostics.h"

#include "settings.h"

#define DIAG_LOG_SIZE 24
#define SCAN_STEP_INTERVAL_MS 100

static DiagStatus status;
static int16_t scanValues[256];
static String logLines[DIAG_LOG_SIZE];
static uint8_t logHead = 0, logCount = 0;
static unsigned long nextStepAt = 0;

// On/off test phases
enum OnOffPhase : uint8_t
{
    PH_ON_SENT,
    PH_OFF_SENT
};
static OnOffPhase onOffPhase;
static uint16_t onOffSeconds;
static unsigned long phaseStart;
static unsigned long lastStateRead;

void diagLog(const String &line)
{
    char prefix[16];
    snprintf(prefix, sizeof(prefix), "[%lus] ", millis() / 1000);
    logLines[(logHead + logCount) % DIAG_LOG_SIZE] = String(prefix) + line;
    if (logCount < DIAG_LOG_SIZE)
        logCount++;
    else
        logHead = (logHead + 1) % DIAG_LOG_SIZE;
    Serial.printf("[diag] %s\n", line.c_str());
}

uint8_t diagLogCount() { return logCount; }
const String &diagLogLine(uint8_t index) { return logLines[(logHead + index) % DIAG_LOG_SIZE]; }

void diagBegin()
{
    memset(&status, 0, sizeof(status));
    for (int i = 0; i < 256; i++)
        scanValues[i] = -1;
}

bool diagBusy() { return status.running != DIAG_IDLE; }
const DiagStatus &diagStatus() { return status; }
int16_t diagScanValue(uint8_t address) { return scanValues[address]; }

bool diagStartScan(uint8_t memory, uint8_t start, uint8_t end)
{
    if (diagBusy() || end < start)
        return false;
    for (int i = 0; i < 256; i++)
        scanValues[i] = -1;
    status.running = status.last = DIAG_SCAN;
    status.memory = memory;
    status.start = start;
    status.end = end;
    status.current = start;
    status.okCount = status.errorCount = 0;
    status.startedAt = millis();
    nextStepAt = millis();
    diagLog(String(F("Scan started: ")) + (memory == MEM_EEPROM ? "EEPROM" : "RAM") + " 0x" + String(start, HEX) +
            "-0x" + String(end, HEX));
    return true;
}

bool diagStartOnOff(uint16_t onSeconds)
{
    if (diagBusy())
        return false;
    status.running = status.last = DIAG_ONOFF;
    status.startedAt = millis();
    onOffSeconds = constrain(onSeconds, 1, 600);
    stoveTurnOn(true);
    onOffPhase = PH_ON_SENT;
    phaseStart = lastStateRead = millis();
    diagLog(String(F("On/off test: ON sent, OFF in ")) + onOffSeconds + "s");
    return true;
}

void diagStop()
{
    if (status.running == DIAG_ONOFF && onOffPhase == PH_ON_SENT)
    {
        stoveTurnOff(true);
        diagLog(F("On/off test stopped: OFF sent"));
    }
    else if (status.running == DIAG_SCAN)
        diagLog(String(F("Scan stopped at 0x")) + String(status.current, HEX));
    status.running = DIAG_IDLE;
}

static void logState()
{
    if (stoveReadMetric(M_STATE))
        diagLog(String(F("State: ")) + stoveData.state + " (" + stoveStateName(stoveData.state) + ")");
    else
        diagLog(F("State: no valid reply"));
}

static void scanStep()
{
    if ((long)(millis() - nextStepAt) < 0)
        return;
    uint8_t value;
    ReadResult r = stoveReadRegister(readCodeFor(status.memory), (uint8_t)status.current, value);
    if (r == READ_OK)
    {
        scanValues[status.current] = value;
        status.okCount++;
    }
    else
        status.errorCount++;

    if (status.current >= status.end)
    {
        status.running = DIAG_IDLE;
        diagLog(String(F("Scan finished: ")) + status.okCount + " ok, " + status.errorCount + " errors, " +
                (millis() - status.startedAt) / 1000 + "s");
        return;
    }
    status.current++;
    nextStepAt = millis() + SCAN_STEP_INTERVAL_MS;
}

static void onOffStep()
{
    unsigned long now = millis();
    if (now - lastStateRead >= 2000) // Log the state every 2 s so the user sees the transitions
    {
        lastStateRead = now;
        logState();
    }
    if (onOffPhase == PH_ON_SENT && now - phaseStart >= onOffSeconds * 1000UL)
    {
        stoveTurnOff(true);
        diagLog(F("OFF sent"));
        onOffPhase = PH_OFF_SENT;
        phaseStart = now;
    }
    else if (onOffPhase == PH_OFF_SENT && now - phaseStart >= 6000)
    {
        logState();
        status.running = DIAG_IDLE;
        diagLog(F("On/off test finished"));
        requestPoll(0);
    }
}

void diagLoop()
{
    switch (status.running)
    {
    case DIAG_SCAN:
        scanStep();
        break;
    case DIAG_ONOFF:
        onOffStep();
        break;
    default:
        break;
    }
}
