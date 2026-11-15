#include "mqtt.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "settings.h"
#include "stove.h"
#include "network.h"

static WiFiClient espClient;
static PubSubClient client(espClient);
static unsigned long lastAttempt = 0;
static bool deepSleep = false;

static String topic(const char *suffix)
{
    return String(settings.mqttTopic) + "/" + suffix;
}

bool mqttEnabled() { return settings.mqttServer[0] != '\0'; }
bool mqttConnected() { return client.connected(); }
int mqttState() { return client.state(); }
bool mqttDeepSleepRequested() { return deepSleep; }

static String deviceInfo()
{
    return String(F("{\"manufacturer\": \"Philibert Cheminot\",\"identifiers\": [\"7a396f39-80d2-493b-8e8e-31a70e700bc6\"],"
                    "\"model\": \"Micronova Controller\",\"name\": \"Micronova Controller\",\"sw_version\": \"" FIRMWARE_VERSION "\","
                    "\"configuration_url\": \"http://")) +
           WiFi.localIP().toString() + "/\"}";
}

static void publishSensorDiscovery(const char *object, const char *name, const char *stateSuffix,
                                   const char *extra, const char *uniqueId)
{
    String t = String(F("homeassistant/sensor/Micronova/")) + object + "/config";
    String p = String(F("{\"name\": \"")) + name + F("\", \"state_topic\": \"") + topic(stateSuffix) +
               F("\", \"qos\": 0, ") + extra + F(", \"unique_id\": \"") + uniqueId +
               F("\", \"device\": ") + deviceInfo() + "}";
    client.publish(t.c_str(), p.c_str(), true);
}

// Same entities and unique IDs as firmware 1.x so existing Home Assistant setups keep working
static void publishDiscovery()
{
    String t = F("homeassistant/switch/Micronova/Controller/config");
    String p = String(F("{\"name\": \"Controller\", \"command_topic\": \"")) + topic("intopic") +
               F("\", \"state_topic\": \"") + topic("onoff") +
               F("\", \"payload_on\": \"ON\", \"payload_off\": \"OFF\", \"state_on\": \"ON\", \"state_off\": \"OFF\", "
                 "\"retain\": false, \"optimistic\": false, \"qos\": 0, \"icon\": \"mdi:fire\", "
                 "\"unique_id\": \"d5668e9c-843c-4330-ae53-a5bd135a4412\", \"device\": ") +
               deviceInfo() + "}";
    client.publish(t.c_str(), p.c_str(), true);

    const char *temp = "\"device_class\": \"temperature\", \"state_class\": \"measurement\", \"unit_of_measurement\": \"°C\"";
    publishSensorDiscovery("Controller", "Controller", "pong", "\"icon\": \"mdi:power\"", "0038ec49-3921-4f1b-9fa9-71acb04052fa");
    publishSensorDiscovery("Temperature", "Temperature", "ambtemp", (String(temp) + ", \"icon\": \"mdi:thermometer\"").c_str(),
                           "9db3245e-6ace-4d14-ac07-844cd68d245c");
    publishSensorDiscovery("FumesTemperature", "Fumes Temperature", "fumetemp",
                           (String(temp) + ", \"icon\": \"mdi:thermometer\"").c_str(), "3c72e1cf-bc22-499e-9f85-7c7e960f9a95");
    publishSensorDiscovery("State", "State", "state", "\"icon\": \"mdi:fire-alert\"", "62fe5080-7668-409b-8451-323364b42eff");
    publishSensorDiscovery("FlamePower", "Flame Power", "flamepower", "\"unit_of_measurement\": \"%\", \"icon\": \"mdi:fire\"",
                           "c3aecb86-66e2-4358-bccb-e3b620f3d28b");
    if (settings.hydroMode)
    {
        publishSensorDiscovery("WaterTemperature", "Water Temperature", "watertemp",
                               (String(temp) + ", \"icon\": \"mdi:coolant-temperature\"").c_str(),
                               "7eb9a6b2-8e26-49f0-b75a-dd97f537d856");
        publishSensorDiscovery("WaterPressure", "Water Pressure", "waterpres",
                               "\"device_class\": \"pressure\", \"unit_of_measurement\": \"bar\", \"icon\": \"mdi:gauge\"",
                               "2e272bb3-7b55-4867-bf1a-b8772d0ae90d");
    }
    Serial.println(F("[mqtt] Home Assistant discovery published"));
}

void mqttPublishMetrics()
{
    if (!client.connected())
        return;
    for (uint8_t i = 0; i < METRIC_COUNT; i++)
    {
        MetricId id = (MetricId)i;
        if (!metricActive(id) || !stoveData.valid[id])
            continue;
        client.publish(topic(METRIC_INFO[id].key).c_str(), formatMetric(id).c_str(), true);
    }
    if (stoveData.state >= 0)
        client.publish(topic("onoff").c_str(), stoveStateIsOn(stoveData.state) ? "ON" : "OFF", true);
    client.publish(topic("pong").c_str(), "Connected", true);
}

String executeCommand(const String &raw)
{
    String cmd = raw;
    cmd.trim();
    String lower = cmd;
    lower.toLowerCase();
    String result;

    if (lower == "on" || lower == "1")
        result = stoveTurnOn() ? F("Stove turned on") : F("Ignored: stove already on");
    else if (lower == "off" || lower == "0")
        result = stoveTurnOff() ? F("Stove turned off") : F("Ignored: stove already off");
    else if (lower.startsWith("fo")) // "force off"
    {
        stoveForceOff();
        result = F("Force off sent");
    }
    else if (cmd.length() > 1 && (cmd[0] == 'T' || cmd[0] == 't'))
    {
        float t = cmd.substring(1).toFloat();
        result = stoveSetTemperature(t) ? String(F("Temperature set to ")) + String(t, 1) : String(F("Invalid temperature"));
    }
    else if (cmd.length() > 1 && (cmd[0] == 'P' || cmd[0] == 'p'))
    {
        int pw = cmd.substring(1).toInt();
        result = stoveSetPower(pw) ? String(F("Power set to ")) + pw : String(F("Invalid power"));
    }
    else if (lower == "s")
    {
        deepSleep = true;
        result = F("Deep sleep enabled");
    }
    else if (lower == "w")
    {
        deepSleep = false;
        result = F("Deep sleep disabled");
    }
    else if (lower == "reset")
    {
        settingsClearWifi();
        netScheduleRestart(1000);
        result = F("WiFi credentials erased, restarting into setup mode");
    }
    else
        result = String(F("Unknown command: ")) + cmd;

    Serial.printf("[cmd] %s -> %s\n", cmd.c_str(), result.c_str());
    requestPoll(1500); // Give the stove some time, then refresh the values
    return result;
}

static void callback(char *t, byte *payload, unsigned int length)
{
    String msg;
    msg.reserve(length);
    for (unsigned int i = 0; i < length; i++)
        msg += (char)payload[i];
    Serial.printf("[mqtt] Message arrived [%s] %s\n", t, msg.c_str());
    executeCommand(msg);
}

static void tryConnect()
{
    String clientId = String(F("ESPClient-")) + String(random(0xffff), HEX);
    String will = topic("pong");
    const char *user = settings.mqttUser[0] ? settings.mqttUser : nullptr;
    const char *pass = settings.mqttPass[0] ? settings.mqttPass : nullptr;
    Serial.printf("[mqtt] Connecting to %s:%u...", settings.mqttServer, settings.mqttPort);
    if (client.connect(clientId.c_str(), user, pass, will.c_str(), 0, true, "Disconnected"))
    {
        Serial.println(F(" connected"));
        client.subscribe(topic("intopic").c_str());
        publishDiscovery();
        mqttPublishMetrics();
    }
    else
        Serial.printf(" failed, rc=%d\n", client.state());
}

void mqttBegin()
{
    if (!mqttEnabled())
    {
        Serial.println(F("[mqtt] No server configured, MQTT disabled"));
        return;
    }
    espClient.setTimeout(2000); // Keep a missing broker from blocking the web interface for long
    client.setSocketTimeout(2);
    client.setBufferSize(1024);
    client.setServer(settings.mqttServer, settings.mqttPort);
    client.setCallback(callback);
}

void mqttLoop()
{
    if (!mqttEnabled() || !netStaConnected())
        return;
    if (!client.connected())
    {
        if (lastAttempt == 0 || millis() - lastAttempt >= MQTT_RETRY_INTERVAL_MS)
        {
            lastAttempt = millis();
            tryConnect();
        }
        return;
    }
    client.loop();
}
