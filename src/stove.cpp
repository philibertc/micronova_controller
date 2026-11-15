#include "stove.h"

#include <SoftwareSerial.h>
#include "settings.h"

static SoftwareSerial StoveSerial;
StoveData stoveData;

const char *readResultName(ReadResult r)
{
    switch (r)
    {
    case READ_OK:
        return "ok";
    case READ_NO_REPLY:
        return "no reply";
    case READ_BAD_LENGTH:
        return "bad length";
    case READ_BAD_CHECKSUM:
        return "bad checksum";
    }
    return "?";
}

void stoveBegin()
{
    pinMode(ENABLE_RX, OUTPUT);
    digitalWrite(ENABLE_RX, HIGH); // The LED of the optocoupler is off
    StoveSerial.begin(STOVE_BAUD, STOVE_SERIAL_MODE, RX_PIN, TX_PIN, false, 256);
    memset(&stoveData, 0, sizeof(stoveData));
    stoveData.state = -1;
}

ReadResult stoveReadRegister(uint8_t code, uint8_t address, uint8_t &value, uint8_t *rawReply, uint8_t *rawLen)
{
    while (StoveSerial.available()) // Drop any leftover byte
        StoveSerial.read();

    StoveSerial.write(code);
    delay(1);
    StoveSerial.write(address);
    digitalWrite(ENABLE_RX, LOW);
    delay(STOVE_REPLY_DELAY_MS);

    uint8_t rx[2] = {0, 0};
    uint8_t count = 0;
    while (StoveSerial.available()) // It has to be exactly 2 bytes, otherwise it's an error
    {
        uint8_t b = StoveSerial.read();
        if (count < 2)
            rx[count] = b;
        count++;
    }
    digitalWrite(ENABLE_RX, HIGH);

    if (rawReply)
    {
        rawReply[0] = rx[0];
        rawReply[1] = rx[1];
    }
    if (rawLen)
        *rawLen = count;

    ReadResult result;
    if (count == 0)
        result = READ_NO_REPLY;
    else if (count != 2)
        result = READ_BAD_LENGTH;
    else if ((uint8_t)(code + address + rx[1]) != rx[0]) // Reply = [checksum, value]
        result = READ_BAD_CHECKSUM;
    else
    {
        value = rx[1];
        result = READ_OK;
    }

    if (result == READ_OK)
        stoveData.readOk++;
    else
        stoveData.readErrors++;
    return result;
}

void stoveWriteRegister(uint8_t code, uint8_t address, uint8_t value)
{
    const uint8_t frame[4] = {code, address, value, (uint8_t)(code + address + value)};
    Serial.printf("[stove] write %02X %02X %02X %02X\n", frame[0], frame[1], frame[2], frame[3]);
    for (uint8_t i = 0; i < 4; i++)
    {
        StoveSerial.write(frame[i]);
        delay(1);
    }
}

bool metricActive(MetricId id)
{
    if (!settings.metrics[id].enabled)
        return false;
    if (METRIC_INFO[id].hydroOnly && !settings.hydroMode)
        return false;
    return true;
}

float metricRawToValue(MetricId id, uint8_t raw)
{
    if (id == M_STATE)
        return raw;
    if (id == M_FLAME_POWER) // 0..16 mapped to 10..100 %, 0 when the stove is not burning
        return stoveStateIsOn(stoveData.state) ? map(raw, 0, 16, 10, 100) : 0;
    uint8_t divisor = settings.metrics[id].divisor ? settings.metrics[id].divisor : 1;
    return (float)raw / divisor;
}

bool stoveReadMetric(MetricId id)
{
    const MetricConfig &cfg = settings.metrics[id];
    uint8_t raw;
    ReadResult r = stoveReadRegister(readCodeFor(cfg.memory), cfg.address, raw);
    if (r != READ_OK)
    {
        Serial.printf("[stove] %s @%02X: %s\n", METRIC_INFO[id].key, cfg.address, readResultName(r));
        return false;
    }
    if (id == M_STATE)
        stoveData.state = raw;
    stoveData.raw[id] = raw;
    stoveData.value[id] = metricRawToValue(id, raw);
    stoveData.valid[id] = true;
    stoveData.lastUpdate[id] = millis();
    return true;
}

void stovePollAll()
{
    for (uint8_t i = 0; i < METRIC_COUNT; i++)
    {
        if (!metricActive((MetricId)i))
            continue;
        stoveReadMetric((MetricId)i);
        delay(20);
        yield();
    }
    stoveData.lastPoll = millis();
}

static void writeState(uint8_t value)
{
    const MetricConfig &cfg = settings.metrics[M_STATE];
    stoveWriteRegister(writeCodeFor(cfg.memory), cfg.address, value);
}

bool stoveTurnOn(bool force)
{
    int s = stoveData.state;
    if (!force && !(s == 0 || s > 5))
        return false;
    writeState(STOVE_CMD_ON);
    return true;
}

bool stoveTurnOff(bool force)
{
    int s = stoveData.state;
    if (!force && !(s > 0 && s < 6))
        return false;
    writeState(STOVE_CMD_OFF);
    return true;
}

void stoveForceOff()
{
    writeState(STOVE_CMD_FORCE_OFF);
}

bool stoveSetTemperature(float celsius)
{
    if (celsius < 0 || celsius > 60)
        return false;
    const MetricConfig &cfg = settings.metrics[M_TEMP_SET];
    long raw = lroundf(celsius * (cfg.divisor ? cfg.divisor : 1));
    if (raw > 255)
        return false;
    stoveWriteRegister(writeCodeFor(cfg.memory), cfg.address, (uint8_t)raw);
    return true;
}

bool stoveSetPower(uint8_t power)
{
    if (power < 1 || power > 9)
        return false;
    const MetricConfig &cfg = settings.metrics[M_POWER_SET];
    stoveWriteRegister(writeCodeFor(cfg.memory), cfg.address, power);
    return true;
}

String formatMetric(MetricId id)
{
    if (id == M_STATE)
        return stoveStateName(stoveData.state);
    uint8_t divisor = settings.metrics[id].divisor;
    if (id == M_FLAME_POWER || divisor <= 1)
        return String((int)lroundf(stoveData.value[id]));
    return String(stoveData.value[id], divisor <= 10 ? 1 : 2);
}

// Poll scheduling ---------------------------------------------------------------
static unsigned long nextPollAt = 0; // 0 = poll as soon as possible

void requestPoll(unsigned long delayMs)
{
    nextPollAt = (millis() + delayMs) | 1;
}

bool pollDue()
{
    if (nextPollAt == 0 || (long)(millis() - nextPollAt) >= 0)
    {
        nextPollAt = (millis() + settings.pollInterval * 1000UL) | 1;
        return true;
    }
    return false;
}
