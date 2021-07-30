#ifdef ESP8266
#include <FS.h>
#include <SoftwareSerial.h>
SoftwareSerial StoveSerial;
#define SERIAL_MODE SWSERIAL_8N2 //8 data bits, parity none, 2 stop bits
#endif

#ifdef ESP32
#include <SPIFFS.h>
#include <HardwareSerial.h>
HardwareSerial StoveSerial(1);
#define SERIAL_MODE SERIAL_8N2
#endif

#include <WiFiManager.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wm;
WiFiManagerParameter custom_mqtt_server("server", "mqtt_server", "", 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt_port", "1883", 40);
WiFiManagerParameter custom_base_topic("topic", "base_topic", "micronova", 40);
WiFiManagerParameter custom_mqtt_user("user", "mqtt_user", "", 40);
WiFiManagerParameter custom_mqtt_pass("pass", "mqtt_pass", "", 40);

String server;
String port;
String topic;
String user;
char *mqtt_user;
String pass;
char *mqtt_pass;
String ambtemp_topic;
char *char_ambtemp_topic;
String fumetemp_topic;
char *char_fumetemp_topic;
String state_topic;
char *char_state_topic;
String onoff_topic;
char *char_onoff_topic;
String in_topic;
char *char_in_topic;

#define serialPin 16

void saveParamsCallback()
{
    Serial.println("Get Params:");
    Serial.println(custom_mqtt_server.getValue());
    Serial.println(custom_mqtt_port.getValue());
    Serial.println(custom_base_topic.getValue());
    Serial.println(custom_mqtt_user.getValue());
    Serial.println(custom_mqtt_pass.getValue());
    Serial.println(F("Initializing FS..."));
    File file = SPIFFS.open("/config.txt", "a+"); //We open a file to save value on SPIFFS
    file.println(custom_mqtt_server.getValue());  //1st line: MQTT server address
    file.println(custom_mqtt_port.getValue());    //2nd line: MQTT port
    file.println(custom_base_topic.getValue());   //3rd line: base topic
    file.println(custom_mqtt_user.getValue());    //4th line: MQTT username
    file.println(custom_mqtt_pass.getValue());    //5th line: MQTT password
}

void setup_wifi()
{
    WiFi.mode(WIFI_STA);
    wm.addParameter(&custom_mqtt_server);
    wm.addParameter(&custom_mqtt_port);
    wm.addParameter(&custom_base_topic);
    wm.addParameter(&custom_mqtt_user);
    wm.addParameter(&custom_mqtt_pass);
    wm.setSaveParamsCallback(saveParamsCallback); //Saves the settings in SPIFFS
    wm.autoConnect("Pellet Stove Controller");
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
        SPIFFS.format();
        ESP.restart();
    }
}

void setup()
{
    Serial.begin(115200);
    StoveSerial.begin(1200, SERIAL_MODE, serialPin, serialPin, false, 256);
    if (SPIFFS.begin())
    {
        Serial.println(F("SPIFFS system mounted with success"));
    }
    else
    {
        Serial.println(F("An Error has occurred while mounting SPIFFS"));
    }
    setup_wifi();
    int line = 0;
    File configFile = SPIFFS.open("/config.txt", "r");
    while (configFile.available())
    {
        if (line == 0)
        {
            String serverString = configFile.readStringUntil('\n');
            server = serverString.c_str();
            server.trim();
        }
        if (line == 1)
        {
            String portString = configFile.readStringUntil('\n');
            port = portString.c_str();
            port.trim();
        }
        if (line == 2)
        {
            String topicString = configFile.readStringUntil('\n');
            topic = topicString.c_str();
            topic.trim();
            ambtemp_topic += topic;
            fumetemp_topic += topic;
            state_topic += topic;
            onoff_topic += topic;
            in_topic += topic;
            ambtemp_topic += "/ambtemp";
            fumetemp_topic += "/fumetemp";
            state_topic += "/state";
            onoff_topic += "/onoff";
            in_topic += "/intopic";
            ambtemp_topic.toCharArray(char_ambtemp_topic, 50);
            fumetemp_topic.toCharArray(char_fumetemp_topic, 50);
            state_topic.toCharArray(char_state_topic, 50);
            onoff_topic.toCharArray(char_onoff_topic, 50);
            in_topic.toCharArray(char_in_topic, 50);
        }
        if (line == 3)
        {
            String userString = configFile.readStringUntil('\n');
            user = userString.c_str();
            user.trim();
            user.toCharArray(mqtt_user, 50);
        }
        if (line == 4)
        {
            String passString = configFile.readStringUntil('\n');
            pass = passString.c_str();
            pass.trim();
            user.toCharArray(mqtt_pass, 50);
        }
        line++;
    }
    /*client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    client.subscribe(in_topic);*/
}

void loop()
{
    /*if (!client.connected())
    {
        reconnect();
        client.subscribe(char_in_topic);
    }
    client.loop();*/
}
