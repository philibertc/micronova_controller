#include "web_server.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "config.h"
#include "settings.h"
#include "stove.h"
#include "network.h"
#include "mqtt.h"
#include "diagnostics.h"
#include "web_ui.h"

static ESP8266WebServer server(80);

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// JSON string literal with escaping
static String js(const char *s)
{
    String out;
    out.reserve(strlen(s) + 2);
    out += '"';
    for (; *s; s++)
    {
        char c = *s;
        if (c == '"' || c == '\\')
        {
            out += '\\';
            out += c;
        }
        else if ((uint8_t)c < 0x20)
        {
            char buf[8];
            snprintf(buf, sizeof(buf), "\\u%04x", c);
            out += buf;
        }
        else
            out += c;
    }
    out += '"';
    return out;
}
static String js(const String &s) { return js(s.c_str()); }

static void sendJson(const String &body, int code = 200)
{
    server.sendHeader(F("Cache-Control"), F("no-store"));
    server.send(code, F("application/json"), body);
}

static void sendResult(bool ok, const String &message, int code = 200)
{
    sendJson(String(F("{\"ok\":")) + (ok ? "true" : "false") + F(",\"message\":") + js(message) + "}", code);
}

static void sendError(const String &message) { sendResult(false, message, 400); }

// Accepts decimal or 0x-prefixed hexadecimal
static long argNumber(const char *name, long fallback)
{
    if (!server.hasArg(name) || server.arg(name).length() == 0)
        return fallback;
    return strtol(server.arg(name).c_str(), nullptr, 0);
}

static bool argBool(const char *name)
{
    String v = server.arg(name);
    return v == "1" || v == "true" || v == "on";
}

static int metricByKey(const String &key)
{
    for (uint8_t i = 0; i < METRIC_COUNT; i++)
        if (key == METRIC_INFO[i].key)
            return i;
    return -1;
}

// Authentication is skipped in setup mode (no WiFi credentials): whoever triggered RESET_PIN has
// physical access to the board, and may not remember the password.
static bool authorized()
{
    if (settings.webPass[0] == '\0' || netMode() == NET_SETUP)
        return true;
    if (server.authenticate(settings.webUser, settings.webPass))
        return true;
    server.requestAuthentication(BASIC_AUTH, "Micronova Controller");
    return false;
}

#define ROUTE(path, method, handler) \
    server.on(F(path), method, []() { if (authorized()) handler(); })

// ---------------------------------------------------------------------------
// Status
// ---------------------------------------------------------------------------

static const char *netModeName()
{
    switch (netMode())
    {
    case NET_SETUP:
        return "setup";
    case NET_CONNECTING:
        return "connecting";
    case NET_CONNECTED:
        return "connected";
    }
    return "?";
}

static String metricJson(MetricId id)
{
    const MetricInfo &info = METRIC_INFO[id];
    const MetricConfig &cfg = settings.metrics[id];
    char addr[8];
    snprintf(addr, sizeof(addr), "0x%02X", cfg.address);
    String s = String(F("{\"key\":")) + js(info.key) + F(",\"label\":") + js(info.label) + F(",\"unit\":") + js(info.unit) +
               F(",\"address\":\"") + addr + F("\",\"memory\":") + cfg.memory + F(",\"divisor\":") + cfg.divisor +
               F(",\"enabled\":") + (cfg.enabled ? "true" : "false") + F(",\"active\":") +
               (metricActive(id) ? "true" : "false") + F(",\"hydro\":") + (info.hydroOnly ? "true" : "false");
    snprintf(addr, sizeof(addr), "0x%02X", info.defaultAddress);
    s += String(F(",\"default\":{\"address\":\"")) + addr + F("\",\"memory\":") + info.defaultMemory +
         F(",\"divisor\":") + info.defaultDivisor + "}";
    if (stoveData.valid[id])
        s += String(F(",\"valid\":true,\"raw\":")) + stoveData.raw[id] + F(",\"value\":") + js(formatMetric(id)) +
             F(",\"age\":") + (millis() - stoveData.lastUpdate[id]) / 1000;
    else
        s += F(",\"valid\":false");
    return s + "}";
}

static void handleStatus()
{
    String s;
    s.reserve(2600);
    s = String(F("{\"firmware\":\"" FIRMWARE_VERSION "\",\"uptime\":")) + millis() / 1000 + F(",\"heap\":") + ESP.getFreeHeap();
    s += String(F(",\"net\":{\"mode\":\"")) + netModeName() + F("\",\"ssid\":") + js(settings.wifiSsid) + F(",\"ip\":\"") +
         (netStaConnected() ? WiFi.localIP().toString() : String()) + F("\",\"rssi\":") +
         (netStaConnected() ? WiFi.RSSI() : 0) + F(",\"ap\":") + (netApActive() ? "true" : "false") +
         F(",\"apSsid\":") + js(netApSsid()) + F(",\"hostname\":") + js(settings.hostname) + "}";
    s += String(F(",\"mqtt\":{\"enabled\":")) + (mqttEnabled() ? "true" : "false") + F(",\"connected\":") +
         (mqttConnected() ? "true" : "false") + F(",\"state\":") + mqttState() + F(",\"server\":") +
         js(settings.mqttServer) + "}";
    s += String(F(",\"stove\":{\"state\":")) + stoveData.state + F(",\"stateName\":") + js(stoveStateName(stoveData.state)) +
         F(",\"on\":") + (stoveStateIsOn(stoveData.state) ? "true" : "false") + F(",\"lastPoll\":") +
         (stoveData.lastPoll ? (long)((millis() - stoveData.lastPoll) / 1000) : -1L) + F(",\"readOk\":") +
         stoveData.readOk + F(",\"readErrors\":") + stoveData.readErrors + "}";
    s += String(F(",\"hydro\":")) + (settings.hydroMode ? "true" : "false") + F(",\"deepSleep\":") +
         (mqttDeepSleepRequested() ? "true" : "false") + F(",\"testBusy\":") + (diagBusy() ? "true" : "false") +
         F(",\"metrics\":[");
    for (uint8_t i = 0; i < METRIC_COUNT; i++)
    {
        if (i)
            s += ',';
        s += metricJson((MetricId)i);
    }
    s += "]}";
    sendJson(s);
}

// ---------------------------------------------------------------------------
// Control
// ---------------------------------------------------------------------------

static void handleCommand()
{
    if (!server.hasArg("cmd"))
        return sendError(F("Missing cmd"));
    if (diagBusy())
        return sendError(F("A test is running, stop it first"));
    String result = executeCommand(server.arg("cmd"));
    sendResult(!result.startsWith(F("Unknown")) && !result.startsWith(F("Invalid")), result);
}

static void handlePoll()
{
    if (diagBusy())
        return sendError(F("A test is running"));
    stovePollAll();
    mqttPublishMetrics();
    handleStatus();
}

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

static void handleGetSettings()
{
    String s = String(F("{\"wifi\":{\"ssid\":")) + js(settings.wifiSsid) + F(",\"hasPass\":") +
               (settings.wifiPass[0] ? "true" : "false") + "}";
    s += String(F(",\"mqtt\":{\"server\":")) + js(settings.mqttServer) + F(",\"port\":") + settings.mqttPort +
         F(",\"user\":") + js(settings.mqttUser) + F(",\"hasPass\":") + (settings.mqttPass[0] ? "true" : "false") +
         F(",\"topic\":") + js(settings.mqttTopic) + "}";
    s += String(F(",\"general\":{\"hostname\":")) + js(settings.hostname) + F(",\"poll\":") + settings.pollInterval +
         F(",\"hydro\":") + (settings.hydroMode ? "true" : "false") + F(",\"webuser\":") + js(settings.webUser) +
         F(",\"auth\":") + (settings.webPass[0] ? "true" : "false") + "}}";
    sendJson(s);
}

static void handleSaveWifi()
{
    String ssid = server.arg("ssid");
    ssid.trim();
    if (ssid.length() == 0 || ssid.length() > 32)
        return sendError(F("SSID must be 1 to 32 characters"));
    String pass = server.arg("pass");
    if (pass.length() > 64 || (pass.length() > 0 && pass.length() < 8))
        return sendError(F("Password must be 8 to 64 characters (or empty for an open network)"));
    // An empty password keeps the current one when the SSID does not change
    if (pass.length() > 0 || ssid != settings.wifiSsid)
        COPY_SETTING(wifiPass, pass);
    COPY_SETTING(wifiSsid, ssid);
    if (!settingsSave())
        return sendError(F("Could not save the settings"));
    sendResult(true, String(F("Saved. Restarting and connecting to ")) + ssid + F("..."));
    netScheduleRestart(1500);
}

static void handleSaveMqtt()
{
    String srv = server.arg("server");
    srv.trim();
    String topic = server.arg("topic");
    topic.trim();
    while (topic.endsWith("/"))
        topic.remove(topic.length() - 1);
    long port = argNumber("port", DEFAULT_MQTT_PORT);
    if (port <= 0 || port > 65535)
        return sendError(F("Invalid port"));
    if (topic.length() == 0)
        return sendError(F("Topic cannot be empty"));
    COPY_SETTING(mqttServer, srv);
    settings.mqttPort = port;
    COPY_SETTING(mqttUser, server.arg("user"));
    if (server.arg("pass").length() > 0)
        COPY_SETTING(mqttPass, server.arg("pass"));
    if (argBool("clearpass"))
        settings.mqttPass[0] = '\0';
    COPY_SETTING(mqttTopic, topic);
    if (!settingsSave())
        return sendError(F("Could not save the settings"));
    sendResult(true, F("Saved. Restarting..."));
    netScheduleRestart(1500);
}

static void handleSaveGeneral()
{
    String host = server.arg("hostname");
    host.trim();
    if (host.length() == 0 || host.length() > 32)
        return sendError(F("Hostname must be 1 to 32 characters"));
    for (char c : host)
        if (!isalnum(c) && c != '-')
            return sendError(F("Hostname may only contain letters, digits and '-'"));
    long poll = argNumber("poll", settings.pollInterval);
    if (poll < 5 || poll > 3600)
        return sendError(F("Poll interval must be 5 to 3600 s"));
    COPY_SETTING(hostname, host);
    settings.pollInterval = poll;
    settings.hydroMode = argBool("hydro");
    String user = server.arg("webuser");
    user.trim();
    if (user.length())
        COPY_SETTING(webUser, user);
    if (argBool("noauth"))
        settings.webPass[0] = '\0';
    else if (server.arg("webpass").length() > 0)
        COPY_SETTING(webPass, server.arg("webpass"));
    if (!settingsSave())
        return sendError(F("Could not save the settings"));
    sendResult(true, F("Saved. Restarting..."));
    netScheduleRestart(1500);
}

static void handleWifiScan()
{
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_FAILED)
    {
        WiFi.scanNetworks(true, false);
        return sendJson(F("{\"scanning\":true}"));
    }
    if (n == WIFI_SCAN_RUNNING)
        return sendJson(F("{\"scanning\":true}"));
    String s = F("{\"scanning\":false,\"networks\":[");
    bool first = true;
    for (int i = 0; i < n; i++)
    {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0)
            continue;
        if (!first)
            s += ',';
        first = false;
        s += String(F("{\"ssid\":")) + js(ssid) + F(",\"rssi\":") + WiFi.RSSI(i) + F(",\"open\":") +
             (WiFi.encryptionType(i) == ENC_TYPE_NONE ? "true" : "false") + "}";
    }
    s += "]}";
    WiFi.scanDelete(); // Next call starts a new scan
    sendJson(s);
}

// ---------------------------------------------------------------------------
// Metric addresses
// ---------------------------------------------------------------------------

static void handleSaveMetric()
{
    int id = metricByKey(server.arg("key"));
    if (id < 0)
        return sendError(F("Unknown metric"));
    long addr = argNumber("address", -1);
    long mem = argNumber("memory", settings.metrics[id].memory);
    long div = argNumber("divisor", settings.metrics[id].divisor);
    if (addr < 0 || addr > 255)
        return sendError(F("Address must be 0x00 to 0xFF"));
    if (mem != MEM_RAM && mem != MEM_EEPROM)
        return sendError(F("Invalid memory"));
    if (div < 1 || div > 100)
        return sendError(F("Divisor must be 1 to 100"));
    MetricConfig &cfg = settings.metrics[id];
    cfg.address = addr;
    cfg.memory = mem;
    cfg.divisor = div;
    cfg.enabled = server.hasArg("enabled") ? argBool("enabled") : cfg.enabled;
    stoveData.valid[id] = false;
    if (!settingsSave())
        return sendError(F("Could not save the settings"));
    sendResult(true, String(METRIC_INFO[id].label) + F(" saved"));
    requestPoll(0);
}

static void handleResetMetric()
{
    String key = server.arg("key");
    if (key == "all")
    {
        for (uint8_t i = 0; i < METRIC_COUNT; i++)
        {
            settingsResetMetric((MetricId)i);
            stoveData.valid[i] = false;
        }
    }
    else
    {
        int id = metricByKey(key);
        if (id < 0)
            return sendError(F("Unknown metric"));
        settingsResetMetric((MetricId)id);
        stoveData.valid[id] = false;
    }
    settingsSave();
    sendResult(true, F("Defaults restored"));
    requestPoll(0);
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

static String readJson(uint8_t memory, uint8_t address)
{
    uint8_t value = 0, raw[2], len = 0;
    ReadResult r = stoveReadRegister(readCodeFor(memory), address, value, raw, &len);
    char buf[160];
    snprintf(buf, sizeof(buf),
             "{\"address\":\"0x%02X\",\"memory\":%u,\"ok\":%s,\"result\":\"%s\",\"value\":%u,\"reply\":\"%02X %02X\",\"bytes\":%u}",
             address, memory, r == READ_OK ? "true" : "false", readResultName(r), r == READ_OK ? value : 0, raw[0], raw[1],
             len);
    return buf;
}

static bool checkIdle()
{
    if (!diagBusy())
        return true;
    sendError(F("A test is already running"));
    return false;
}

static void handleTestRead()
{
    if (!checkIdle())
        return;
    long addr = argNumber("address", -1);
    if (addr < 0 || addr > 255)
        return sendError(F("Address must be 0x00 to 0xFF"));
    sendJson(readJson(argNumber("memory", MEM_RAM) == MEM_EEPROM ? MEM_EEPROM : MEM_RAM, addr));
}

static void handleTestWrite()
{
    if (!checkIdle())
        return;
    long addr = argNumber("address", -1);
    long value = argNumber("value", -1);
    if (addr < 0 || addr > 255 || value < 0 || value > 255)
        return sendError(F("Address and value must be 0 to 255"));
    uint8_t mem = argNumber("memory", MEM_RAM) == MEM_EEPROM ? MEM_EEPROM : MEM_RAM;
    stoveWriteRegister(writeCodeFor(mem), addr, value);
    char buf[64];
    snprintf(buf, sizeof(buf), "Wrote %u (0x%02X) to %s 0x%02X", (unsigned)value, (unsigned)value,
             mem == MEM_EEPROM ? "EEPROM" : "RAM", (unsigned)addr);
    diagLog(buf);
    sendResult(true, buf);
}

static void handleTestReadAll()
{
    if (!checkIdle())
        return;
    String s = F("{\"results\":[");
    for (uint8_t i = 0; i < METRIC_COUNT; i++)
    {
        MetricId id = (MetricId)i;
        if (i)
            s += ',';
        String r = readJson(settings.metrics[id].memory, settings.metrics[id].address);
        // Also refresh the metric itself so the dashboard shows the new value
        stoveReadMetric(id);
        s += String(F("{\"key\":")) + js(METRIC_INFO[id].key) + F(",\"label\":") + js(METRIC_INFO[id].label) +
             F(",\"text\":") + js(stoveData.valid[id] ? formatMetric(id) : String()) + F(",\"read\":") + r + "}";
        delay(20);
    }
    s += "]}";
    sendJson(s);
}

static void handleTestScan()
{
    long start = argNumber("start", 0), end = argNumber("end", 255);
    if (start < 0 || end > 255 || end < start)
        return sendError(F("Invalid range"));
    uint8_t mem = argNumber("memory", MEM_RAM) == MEM_EEPROM ? MEM_EEPROM : MEM_RAM;
    if (!diagStartScan(mem, start, end))
        return sendError(F("A test is already running"));
    sendResult(true, F("Scan started"));
}

static void handleTestOnOff()
{
    if (!diagStartOnOff(argNumber("seconds", 2)))
        return sendError(F("A test is already running"));
    sendResult(true, F("On/off test started"));
}

static void handleTestStop()
{
    diagStop();
    sendResult(true, F("Stopped"));
}

static void handleTestStatus()
{
    const DiagStatus &st = diagStatus();
    const char *names[] = {"idle", "scan", "onoff"};
    String s;
    s.reserve(3000);
    s = String(F("{\"running\":\"")) + names[st.running] + F("\",\"last\":\"") + names[st.last] + F("\",\"memory\":") +
        st.memory + F(",\"start\":") + st.start + F(",\"end\":") + st.end + F(",\"current\":") + st.current +
        F(",\"ok\":") + st.okCount + F(",\"errors\":") + st.errorCount + F(",\"elapsed\":") +
        (st.startedAt ? (millis() - st.startedAt) / 1000 : 0) + F(",\"scan\":[");
    // Successful reads as [address, value] pairs
    bool first = true;
    for (int a = 0; a < 256; a++)
    {
        int16_t v = diagScanValue(a);
        if (v < 0)
            continue;
        if (!first)
            s += ',';
        first = false;
        s += '[';
        s += a;
        s += ',';
        s += v;
        s += ']';
    }
    s += F("],\"log\":[");
    for (uint8_t i = 0; i < diagLogCount(); i++)
    {
        if (i)
            s += ',';
        s += js(diagLogLine(i));
    }
    s += "]}";
    sendJson(s);
}

// ---------------------------------------------------------------------------
// System
// ---------------------------------------------------------------------------

static void handleReboot()
{
    sendResult(true, F("Restarting..."));
    netScheduleRestart(1000);
}

static void handleForgetWifi()
{
    settingsClearWifi();
    sendResult(true, String(F("WiFi credentials erased. Connect to the \"")) + netApSsid() + F("\" network to set them up again."));
    netScheduleRestart(1500);
}

static void handleFactoryReset()
{
    settingsSetDefaults();
    settingsSave();
    sendResult(true, String(F("Factory settings restored. Connect to the \"")) + netApSsid() + F("\" network."));
    netScheduleRestart(1500);
}

static void handleRoot()
{
    server.sendHeader(F("Cache-Control"), F("no-cache"));
    server.send_P(200, "text/html", WEB_UI_HTML);
}

// Captive portal: redirect every unknown host/page to the interface while the setup AP is active
static void handleNotFound()
{
    if (netApActive())
    {
        String host = server.hostHeader();
        if (host != WiFi.softAPIP().toString() && host != WiFi.localIP().toString() &&
            !host.startsWith(settings.hostname))
        {
            server.sendHeader(F("Location"), String(F("http://")) + WiFi.softAPIP().toString() + "/", true);
            server.send(302, F("text/plain"), "");
            return;
        }
    }
    server.send(404, F("text/plain"), F("Not found"));
}

void webBegin()
{
    ROUTE("/", HTTP_GET, handleRoot);
    ROUTE("/api/status", HTTP_GET, handleStatus);
    ROUTE("/api/command", HTTP_POST, handleCommand);
    ROUTE("/api/poll", HTTP_POST, handlePoll);
    ROUTE("/api/settings", HTTP_GET, handleGetSettings);
    ROUTE("/api/settings/wifi", HTTP_POST, handleSaveWifi);
    ROUTE("/api/settings/mqtt", HTTP_POST, handleSaveMqtt);
    ROUTE("/api/settings/general", HTTP_POST, handleSaveGeneral);
    ROUTE("/api/wifi/scan", HTTP_GET, handleWifiScan);
    ROUTE("/api/metric", HTTP_POST, handleSaveMetric);
    ROUTE("/api/metric/reset", HTTP_POST, handleResetMetric);
    ROUTE("/api/test/read", HTTP_POST, handleTestRead);
    ROUTE("/api/test/write", HTTP_POST, handleTestWrite);
    ROUTE("/api/test/readall", HTTP_POST, handleTestReadAll);
    ROUTE("/api/test/scan", HTTP_POST, handleTestScan);
    ROUTE("/api/test/onoff", HTTP_POST, handleTestOnOff);
    ROUTE("/api/test/stop", HTTP_POST, handleTestStop);
    ROUTE("/api/test/status", HTTP_GET, handleTestStatus);
    ROUTE("/api/reboot", HTTP_POST, handleReboot);
    ROUTE("/api/wifi/forget", HTTP_POST, handleForgetWifi);
    ROUTE("/api/factory-reset", HTTP_POST, handleFactoryReset);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println(F("[web] HTTP server started on port 80"));
}

void webLoop()
{
    server.handleClient();
}
