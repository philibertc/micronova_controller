#include <FS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#ifdef ESP32
  #include <SPIFFS.h>
#endif
SoftwareSerial StoveSerial;
WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wm;

#define concat(first, second) first second
#define mqtt_server "192.168.1.1"
#define mqtt_port "1883"
#define base_topic "micronova"
#define mqtt_user "username"
#define mqtt_pass "password"
#define ambtemp_topic concat(base_topic, "/ambtemp")
#define fumetemp_topic concat(base_topic, "/fumetemp")
#define state_topic concat(base_topic, "/state")
#define in_topic concat(base_topic, "/intopic")
#define onoff_topic concat(base_topic, "/onoff")

#define serialPin 16

bool shouldSaveConfig = false;

void saveConfigCallback()
{
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

void setup_wifi()
{
    if (SPIFFS.begin())
    {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json"))
        {
            //file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile)
            {
                Serial.println("opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                DynamicJsonBuffer jsonBuffer;
                JsonObject &json = jsonBuffer.parseObject(buf.get());
                json.printTo(Serial);
                if (json.success())
                {
                    Serial.println("\nparsed json");
                    strcpy(mqtt_server, json["mqtt_server"]);
                    strcpy(mqtt_port, json["mqtt_port"]);
                    strcpy(mqtt_user, json["mqtt_user"]);
                    strcpy(mqtt_pass, json["mqtt_pass"]);
                    strcpy(base_topic, json["base_topic"]);
                }
                else
                {
                    Serial.println("failed to load json config");
                }
            }
        }
    }
    else
    {
        Serial.println("failed to mount FS");
    }
    WiFiManagerParameter custom_mqtt_server("server", "mqtt_server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt_port", mqtt_port, 6);
    WiFiManagerParameter custom_mqtt_user("user", "mqtt_user", mqtt_user, 20);
    WiFiManagerParameter custom_mqtt_pass("pass", "mqtt_pass", mqtt_pass, 20);
    WiFiManagerParameter custom_base_topic("topic", "base_topic", base_topic, 20);
    wm.setSaveConfigCallback(saveConfigCallback);
    wm.addParameter(&custom_mqtt_server);
    wm.addParameter(&custom_mqtt_port);
    wm.addParameter(&custom_mqtt_user);
    wm.addParameter(&custom_mqtt_pass);
    wm.addParameter(&custom_base_topic);
    WiFi.mode(WIFI_STA);
    wm.autoConnect("Pellet Stove Controller");
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_pass, custom_mqtt_pass.getValue());
    strcpy(base_topic, custom_base_topic.getValue());
    if (shouldSaveConfig)
    {
        Serial.println("saving config");
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.createObject();
        json["mqtt_server"] = mqtt_server;
        json["mqtt_port"] = mqtt_port;
        json["mqtt_user"] = mqtt_user;
        json["mqtt_pass"] = mqtt_pass;
        json["base_topic"] = base_topic;

        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile)
        {
            Serial.println("failed to open config file for writing");
        }

        json.printTo(Serial);
        json.printTo(configFile);
        configFile.close();
        //end save
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("Micronova", mqtt_user, mqtt_pass))
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    if ((char)payload[1] == 'N')
    {
    }
    else if ((char)payload[1] == 'F')
    {
    }
    else if ((char)payload[2] == 's')
    {
        wm.resetSettings();
        ESP.restart();
    }
}

void setup()
{
    Serial.begin(115200);
    StoveSerial.begin(1200, SWSERIAL_8N2, serialPin, serialPin, false, 256);
    setup_wifi();
    /*client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    client.subscribe(in_topic);*/
}

void loop()
{
    /*if (!client.connected())
    {
        reconnect();
        client.subscribe(in_topic);
    }
    client.loop();*/
    delay(5000);
    Serial.println(mqtt_server);
    Serial.println(mqtt_port);
    Serial.println(mqtt_user);
    Serial.println(mqtt_pass);
    Serial.println(base_topic);
    Serial.println(ambtemp_topic);
    Serial.println(fumetemp_topic);
    Serial.println(state_topic);
    Serial.println(onoff_topic);
    Serial.println(in_topic);
}
