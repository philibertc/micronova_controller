#ifdef ESP8266
#include <FS.h>
#include <SoftwareSerial.h>
SoftwareSerial StoveSerial;
#define SERIAL_MODE SWSERIAL_8N2 //8 data bits, parity none, 2 stop bits
#define RESET_PIN D1
#define RX_PIN D3
#define TX_PIN D4
#define ENABLE_RX D2
#endif

#ifdef ESP32
#include <SPIFFS.h>
#include <HardwareSerial.h>
HardwareSerial StoveSerial(1);
#define SERIAL_MODE SERIAL_8N2 //8 data bits, parity none, 2 stop bits
#define RESET_PIN 36
#define RX_PIN 39
#define TX_PIN 34
#endif

#include <WiFiManager.h>
#include <PubSubClient.h>
#include <string>

WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wm;
WiFiManagerParameter custom_mqtt_server("server", "mqtt_server", "", 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt_port", "1883", 10);
WiFiManagerParameter custom_base_topic("topic", "base_topic", "micronova", 40);
WiFiManagerParameter custom_mqtt_user("user", "mqtt_user", "", 40);
WiFiManagerParameter custom_mqtt_pass("pass", "mqtt_pass", "", 40);
WiFiManagerParameter custom_hydro_mode("hydro", "hydro_mode", "0", 5);

unsigned long previousMillis;

String mqtt_server;
char char_mqtt_server[50];
String mqtt_port;
int int_mqtt_port;
String mqtt_topic;
char char_mqtt_topic[50];
String mqtt_user;
char char_mqtt_user[50];
String mqtt_pass;
char char_mqtt_pass[50];
String ambtemp_topic;
char char_ambtemp_topic[50];
String fumetemp_topic;
char char_fumetemp_topic[50];
String watertemp_topic;
char char_watertemp_topic[50];
String state_topic;
char char_state_topic[50];
String power_topic;
char char_power_topic[50];
String onoff_topic;
char char_onoff_topic[50];
String in_topic;
char char_in_topic[50];
String hydro_mode;
int int_hydro_mode;

//This test turns on the stove, waits 2 sec and turns off the stove.

//0 - OFF, 1 - Starting, 2 - Pellet loading, 3 - Ignition, 4 - Work, 5 - Brazier cleaning, 6 - Final cleaning, 7 - Standby, 8 - Pellet missing alarm, 9 - Ignition failure alarm, 10 - Alarms (to be investigated)

//Checksum: Code+Address+Value on hexadecimal calculator

const char stoveOn[4] = {0x80, 0x21, 0x01, 0xA2};
const char stoveOff[4] = {0x80, 0x21, 0x06, 0xA7};
const char stove1[4] = {0x80, 0x34, 0x01, 0xB5};
const char stove2[4] = {0x80, 0x34, 0x02, 0xB6};
const char stove3[4] = {0x80, 0x34, 0x03, 0xB7};
const char stove4[4] = {0x80, 0x34, 0x04, 0xB8};
const char stove5[4] = {0x80, 0x34, 0x05, 0xB9};

#define stoveStatus 0x21
#define ambTemp 0x01
#define fumeTemp 0x5A
#define waterTemp 0x03
#define stovePower 0x34
uint8_t stoveState, tempFumes, tempWater, flamePower;
float tempAmbient;
char stoveRxData[2];

void saveParamsCallback()
{
    Serial.println("Get Params:");
    Serial.println(custom_mqtt_server.getValue());
    Serial.println(custom_mqtt_port.getValue());
    Serial.println(custom_base_topic.getValue());
    Serial.println(custom_mqtt_user.getValue());
    Serial.println(custom_mqtt_pass.getValue());
    Serial.println(custom_hydro_mode.getValue());
    Serial.println(F("Initializing FS..."));
    File file = SPIFFS.open("/config.txt", "a+"); //We open a file to save value on SPIFFS
    file.println(custom_mqtt_server.getValue());  //1st line: MQTT server address
    file.println(custom_mqtt_port.getValue());    //2nd line: MQTT port
    file.println(custom_base_topic.getValue());   //3rd line: base topic
    file.println(custom_mqtt_user.getValue());    //4th line: MQTT username
    file.println(custom_mqtt_pass.getValue());    //5th line: MQTT password
    file.println(custom_hydro_mode.getValue());   //6th line: Hydro mode
}

void setup_wifi()
{
    WiFi.mode(WIFI_STA);
    wm.addParameter(&custom_mqtt_server);
    wm.addParameter(&custom_mqtt_port);
    wm.addParameter(&custom_base_topic);
    wm.addParameter(&custom_mqtt_user);
    wm.addParameter(&custom_mqtt_pass);
    wm.addParameter(&custom_hydro_mode);
    wm.setSaveParamsCallback(saveParamsCallback); //Saves the settings in SPIFFS
    wm.setConnectTimeout(30);
    wm.autoConnect("Pellet Stove Controller");
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESPClient-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str(), char_mqtt_user, char_mqtt_pass))
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

void fullReset()
{
    Serial.println("Resetting…");
    wm.resetSettings();
    SPIFFS.format();
    ESP.restart();
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
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stoveOn[i]);
            delay(1);
        }
    }
    else if ((char)payload[1] == 'F')
    {
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stoveOff[i]);
            delay(1);
        }
    }
    else if ((char)payload[0] == '1')
    {
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stove1[i]);
            delay(1);
        }
    }
    else if ((char)payload[0] == '2')
    {
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stove2[i]);
            delay(1);
        }
    }
    else if ((char)payload[0] == '3')
    {
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stove3[i]);
            delay(1);
        }
    }
    else if ((char)payload[0] == '4')
    {
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stove4[i]);
            delay(1);
        }
    }
    else if ((char)payload[0] == '5')
    {
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stove5[i]);
            delay(1);
        }
    }
    else if ((char)payload[2] == 's')
    {
        fullReset();
    }
}

void checkStoveReply() //Works only when request is RAM
{
    delay(20);
    uint8_t rxCount = 0;
    stoveRxData[0] = 0x00;
    stoveRxData[1] = 0x00;
    while (StoveSerial.available()) //It has to be exactly 2 bytes, otherwise it's an error
    {
        stoveRxData[rxCount] = StoveSerial.read();
        rxCount++;
    }
    digitalWrite(ENABLE_RX, HIGH);
    if (rxCount == 2)
    {
        byte val = stoveRxData[1];
        byte checksum = stoveRxData[0];
        byte param = checksum - val;
        std::string s;
        char const *pchar;
        Serial.printf("Param=%01x value=%01x", param, val);
        switch (param)
        {
        case stoveStatus:
            stoveState = val;
            switch (stoveState)
            {
            case 0:
                client.publish(char_state_topic, "Off");
                delay(1000);
                client.publish(char_onoff_topic, "OFF");
                break;
            case 1:
                client.publish(char_state_topic, "Starting");
                delay(1000);
                client.publish(char_onoff_topic, "ON");
                break;
            case 2:
                client.publish(char_state_topic, "Pellet loading");
                break;
            case 3:
                client.publish(char_state_topic, "Ignition");
                break;
            case 4:
                client.publish(char_state_topic, "Work");
                break;
            case 5:
                client.publish(char_state_topic, "Brazier cleaning");
                break;
            case 6:
                client.publish(char_state_topic, "Final cleaning");
                break;
            case 7:
                client.publish(char_state_topic, "Standby");
                break;
            case 8:
                client.publish(char_state_topic, "Pellet missing");
                break;
            case 9:
                client.publish(char_state_topic, "Ignition failure");
                break;
            case 10:
                client.publish(char_state_topic, "Alarm");
                break;
            }
            Serial.printf("Stove %s\n", stoveState ? "ON" : "OFF");
            break;
        case ambTemp:
            tempAmbient = val / 2;
            client.publish(char_ambtemp_topic, String(tempAmbient).c_str());
            Serial.printf("T. amb. %d\n", tempAmbient);
            break;
        case fumeTemp:
            tempFumes = val;
            client.publish(char_fumetemp_topic, String(tempFumes).c_str());
            Serial.printf("T. fumes %d\n", tempFumes);
            break;
        case waterTemp:
            tempWater = val;
            client.publish(char_watertemp_topic, String(tempWater).c_str());
            Serial.printf("T. water %d\n", tempWater);
            break;
        case stovePower:
            flamePower = val;
            client.publish(char_power_topic, String(flamePower).c_str());
            Serial.printf("Fire %d\n", flamePower);
            break;
        }
    }
}

void getStoveState()
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(stoveStatus);
    digitalWrite(ENABLE_RX, LOW);
    delay(60);
    checkStoveReply();
}

void getStovePower()
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(stovePower);
    digitalWrite(ENABLE_RX, LOW);
    delay(60);
    checkStoveReply();
}

void getAmbTemp()
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(ambTemp);
    digitalWrite(ENABLE_RX, LOW);
    delay(60);
    checkStoveReply();
}

void getFumeTemp()
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(fumeTemp);
    digitalWrite(ENABLE_RX, LOW);
    delay(60);
    checkStoveReply();
}

void getWaterTemp()
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(waterTemp);
    digitalWrite(ENABLE_RX, LOW);
    delay(60);
    checkStoveReply();
}

void getStates()
{

    getStoveState();
    delay(100);
    getStovePower();
    delay(100);
    getAmbTemp();
    delay(100);
    getFumeTemp();
    if (int_hydro_mode == 1)
    {
        getWaterTemp();
    }
}

void setup()
{
    pinMode(ENABLE_RX, OUTPUT);
    digitalWrite(ENABLE_RX, HIGH); //The led of the optocoupler is off
    pinMode(RESET_PIN, INPUT_PULLUP);
    if (digitalRead(RESET_PIN) == LOW)
    {
        fullReset();
    }
    Serial.begin(115200);
    StoveSerial.begin(1200, SERIAL_MODE, RX_PIN, TX_PIN, false, 256);
    if (SPIFFS.begin())
    {
        Serial.println("SPIFFS system mounted with success");
    }
    else
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
    }
    setup_wifi();
    File configFile = SPIFFS.open("/config.txt", "r");
    Serial.println("Reading values");
    int line = 0;
    while (configFile.available())
    {
        String serverString = configFile.readStringUntil('\n');
        String portString = configFile.readStringUntil('\n');
        String topicString = configFile.readStringUntil('\n');
        String userString = configFile.readStringUntil('\n');
        String passString = configFile.readStringUntil('\n');
        String hydroString = configFile.readStringUntil('\n');
        mqtt_server = serverString.c_str();
        mqtt_server.trim();
        mqtt_server.toCharArray(char_mqtt_server, 50);
        mqtt_port = portString.c_str();
        mqtt_port.trim();
        int_mqtt_port = mqtt_port.toInt();
        mqtt_topic = topicString.c_str();
        mqtt_topic.trim();
        mqtt_topic.toCharArray(char_mqtt_topic, 50);
        mqtt_user = userString.c_str();
        mqtt_user.trim();
        mqtt_user.toCharArray(char_mqtt_user, 50);
        mqtt_pass = passString.c_str();
        mqtt_pass.trim();
        mqtt_pass.toCharArray(char_mqtt_pass, 50);
        int_mqtt_port = mqtt_port.toInt();
        hydro_mode = hydroString.c_str();
        hydro_mode.trim();
        int_hydro_mode = hydro_mode.toInt();
    }
    Serial.println("Success");
    Serial.println(char_mqtt_server);
    Serial.println(int_mqtt_port);
    Serial.println(char_mqtt_topic);
    Serial.println(char_mqtt_user);
    Serial.println(char_mqtt_pass);
    Serial.println(int_hydro_mode);
    client.setServer(char_mqtt_server, int_mqtt_port);
    client.setCallback(callback);
    client.subscribe(char_in_topic);
}

void loop()
{
    client.loop();
    if (!client.connected())
    {
        Serial.println("Reconnecting");
        reconnect();
        client.subscribe(char_in_topic);
    }
    unsigned long currentMillis = millis();
    if (previousMillis > currentMillis)
    {
        previousMillis = 0;
    }
    if (currentMillis - previousMillis >= 10000)
    {
        previousMillis = currentMillis;
        getStates();
    }
}
