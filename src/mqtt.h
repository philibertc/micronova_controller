// MQTT client: Home Assistant discovery, metric publishing and command topic.
// Topics and commands are compatible with firmware 1.x (prefix = MQTT topic setting):
//   <prefix>/intopic  <- ON | OFF | 1 | 0 | force off | T<temp> | P<power> | S (deep sleep) | W (wake) | reset
//   <prefix>/state, /onoff, /ambtemp, /tempset, /stovepower, /fumetemp, /flamepower, /watertemp, /waterpres, /pong
#pragma once

#include <Arduino.h>

void mqttBegin();
void mqttLoop();
void mqttPublishMetrics(); // Publishes every valid metric (called after each poll)

bool mqttEnabled();
bool mqttConnected();
int mqttState(); // PubSubClient state code

bool mqttDeepSleepRequested();

// Executes a command using the same syntax as the MQTT command topic. Returns a short result message.
String executeCommand(const String &command);
