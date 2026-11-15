#include "network.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>
#include "config.h"
#include "settings.h"

static DNSServer dnsServer;
static NetMode mode = NET_SETUP;
static bool apActive = false;
static bool servicesStarted = false;
static unsigned long connectStart = 0;
static unsigned long staConnectedAt = 0;
static unsigned long restartAt = 0;
static unsigned long resetPressedAt = 0;

static const IPAddress AP_IP(192, 168, 4, 1);
static const IPAddress AP_MASK(255, 255, 255, 0);

String netApSsid()
{
    char buf[40];
    snprintf(buf, sizeof(buf), SETUP_AP_PREFIX "-%04X", (unsigned)(ESP.getChipId() & 0xFFFF));
    return String(buf);
}

static void startAp()
{
    if (apActive)
        return;
    WiFi.mode(settingsHasWifi() ? WIFI_AP_STA : WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_IP, AP_MASK);
    WiFi.softAP(netApSsid().c_str()); // Open network, like the original WiFiManager portal
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", AP_IP); // Captive portal: every name resolves to us
    apActive = true;
    Serial.printf("[net] Setup access point \"%s\" started, browse http://%s/\n",
                  netApSsid().c_str(), AP_IP.toString().c_str());
}

static void stopAp()
{
    if (!apActive)
        return;
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    apActive = false;
    Serial.println(F("[net] Setup access point stopped"));
}

static void startServices()
{
    if (servicesStarted)
        return;
    servicesStarted = true;

    if (MDNS.begin(settings.hostname))
    {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("[net] mDNS: http://%s.local/\n", settings.hostname);
    }

    ArduinoOTA.setHostname(settings.hostname);
    if (settings.webPass[0])
        ArduinoOTA.setPassword(settings.webPass);
    ArduinoOTA.begin();
}

// Credentials stored by the SDK (e.g. by WiFiManager in firmware 1.x) are imported once,
// so upgrading does not require reconfiguring the WiFi.
static void importSdkCredentials()
{
    struct station_config conf;
    if (!wifi_station_get_config_default(&conf) || conf.ssid[0] == 0)
        return;
    char ssid[33] = {0};
    char pass[65] = {0};
    memcpy(ssid, conf.ssid, 32);
    memcpy(pass, conf.password, 64);
    Serial.printf("[net] Importing WiFi credentials for \"%s\" from the SDK config\n", ssid);
    COPY_SETTING(wifiSsid, String(ssid));
    COPY_SETTING(wifiPass, String(pass));
    settingsSave();
}

static void checkResetPin()
{
    if (digitalRead(RESET_PIN) == LOW)
    {
        if (resetPressedAt == 0)
            resetPressedAt = millis() | 1; // Never 0
        else if (millis() - resetPressedAt >= RESET_PIN_HOLD_MS)
        {
            Serial.println(F("[net] RESET_PIN triggered: erasing WiFi credentials"));
            settingsClearWifi();
            WiFi.disconnect(true);
            delay(200);
            ESP.restart();
        }
    }
    else
    {
        resetPressedAt = 0;
    }
}

void netBegin(bool firstBoot)
{
    pinMode(RESET_PIN, INPUT_PULLUP);

    WiFi.persistent(false); // Credentials live in our settings, not in the SDK flash area
    WiFi.setAutoReconnect(true);
    WiFi.hostname(settings.hostname);

    // RESET_PIN held during boot: erase the credentials straight away
    delay(10);
    if (digitalRead(RESET_PIN) == LOW)
    {
        delay(RESET_PIN_HOLD_MS);
        if (digitalRead(RESET_PIN) == LOW)
        {
            Serial.println(F("[net] RESET_PIN held at boot: erasing WiFi credentials"));
            settingsClearWifi();
        }
    }

    if (firstBoot && !settingsHasWifi())
        importSdkCredentials();

    if (!settingsHasWifi())
    {
        mode = NET_SETUP;
        startAp();
        return;
    }

    mode = NET_CONNECTING;
    WiFi.mode(WIFI_STA);
    WiFi.begin(settings.wifiSsid, settings.wifiPass);
    connectStart = millis();
    Serial.printf("[net] Connecting to \"%s\"...\n", settings.wifiSsid);
}

void netLoop()
{
    checkResetPin();

    if (apActive)
        dnsServer.processNextRequest();

    if (restartAt && (long)(millis() - restartAt) >= 0)
    {
        Serial.println(F("[net] Restarting"));
        delay(100);
        ESP.restart();
    }

    if (mode == NET_SETUP)
        return;

    bool connected = WiFi.status() == WL_CONNECTED;
    if (connected && mode != NET_CONNECTED)
    {
        mode = NET_CONNECTED;
        staConnectedAt = millis();
        Serial.printf("[net] Connected, IP %s\n", WiFi.localIP().toString().c_str());
        startServices();
    }
    else if (!connected && mode == NET_CONNECTED)
    {
        mode = NET_CONNECTING;
        connectStart = millis();
        Serial.println(F("[net] WiFi lost, reconnecting..."));
    }

    if (mode == NET_CONNECTING && !apActive && millis() - connectStart >= WIFI_CONNECT_TIMEOUT_MS)
    {
        Serial.println(F("[net] Could not connect, opening the setup access point (still retrying)"));
        startAp();
    }

    // Fallback AP is closed one minute after the station is back, unless someone is using it
    if (mode == NET_CONNECTED && apActive && millis() - staConnectedAt >= 60000UL &&
        WiFi.softAPgetStationNum() == 0)
        stopAp();

    if (servicesStarted)
    {
        MDNS.update();
        ArduinoOTA.handle();
    }
}

NetMode netMode() { return mode; }
bool netApActive() { return apActive; }
bool netStaConnected() { return WiFi.status() == WL_CONNECTED; }

void netScheduleRestart(unsigned long delayMs)
{
    restartAt = (millis() + delayMs) | 1;
}
