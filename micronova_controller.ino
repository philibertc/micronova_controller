#ifdef ESP8266
#include <FS.h>
#include <SoftwareSerial.h>
SoftwareSerial StoveSerial;
#define SERIAL_MODE SWSERIAL_8N2 //8 data bits, parity none, 2 stop bits
#define RESET_PIN D5
#define RX_PIN D3
#define TX_PIN D4
#define ENABLE_RX D2
#endif

#ifdef ESP32
#include <SPIFFS.h>
#include <HardwareSerial.h>
HardwareSerial StoveSerial(1);
#define SERIAL_MODE SERIAL_8N2 //8 data bits, parity none, 2 stop bits
#define RESET_PIN 25
#define RX_PIN 32
#define TX_PIN 33
#define ENABLE_RX 35
#endif

#include <WiFiManager.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wm;
WiFiManagerParameter custom_mqtt_server("server", "mqtt_server", "", 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt_port", "1883", 10);
WiFiManagerParameter custom_base_topic("topic", "base_topic", "micronova", 40);
WiFiManagerParameter custom_mqtt_user("user", "mqtt_user", "", 40);
WiFiManagerParameter custom_mqtt_pass("pass", "mqtt_pass", "", 40);
WiFiManagerParameter custom_hydro_mode("hydro", "hydro_mode", "0", 2);

long previousMillis;

String mqtt_server;
char char_mqtt_server[50];
String mqtt_port;
int int_mqtt_port;
String mqtt_topic;
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
String flame_topic;
char char_flame_topic[50];
String onoff_topic;
char char_onoff_topic[50];
String in_topic;
char char_in_topic[50];
String hydro_mode;
int int_hydro_mode;

//0 - OFF, 1 - Starting, 2 - Pellet loading, 3 - Ignition, 4 - Work, 5 - Brazier cleaning, 6 - Final cleaning, 7 - Standby, 8 - Pellet missing alarm, 9 - Ignition failure alarm, 10 - Alarms (to be investigated)

//Checksum: Code+Address+Value on hexadecimal calculator

const char stoveOn[4] = {0x80, 0x21, 0x01, 0xA2};
const char stoveOff[4] = {0x80, 0x21, 0x06, 0xA7};
const char stove1[4] = {0x80, 0x34, 0x01, 0xB5};
const char stove2[4] = {0x80, 0x34, 0x02, 0xB6};
const char stove3[4] = {0x80, 0x34, 0x03, 0xB7};
const char stove4[4] = {0x80, 0x34, 0x04, 0xB8};
const char stove5[4] = {0x80, 0x34, 0x05, 0xB9};

#define stoveStateAddr 0x21
#define ambTempAddr 0x01
#define fumesTempAddr 0x5A
#define waterTempAddr 0x03
#define flamePowerAddr 0x34
uint8_t stoveState, fumesTemp, flamePower, waterTemp;
float ambTemp;
char stoveRxData[2]; //When the heating is sending data, it sends two bytes: a checksum and the value

void saveParamsCallback() //Save params to SPIFFS
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

void setup_wifi() //Setup WiFiManager and connect to WiFi
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

void reconnect() //Connect to MQTT server
{
    //Loop until we're reconnected
    while (!client.connected())
    {
        Serial.println(char_mqtt_user);
        Serial.println(char_mqtt_pass);
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESPClient-";
        clientId += String(random(0xffff), HEX); //Random client ID
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

void IRAM_ATTR fullReset() //Reset all the settings but without erasing the program
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
    if ((char *)payload == "ON")
    {
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stoveOn[i]);
            delay(1);
        }
    }
    else if ((char *)payload == "OFF")
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
        Serial.printf("Param=%01x value=%01x", param, val);
        switch (param)
        {
        case stoveStateAddr:
            stoveState = val;
            switch (stoveState)
            {
            case 0:
                client.publish(char_state_topic, "Off", true);
                delay(1000);
                client.publish(char_onoff_topic, "OFF", true);
                break;
            case 1:
                client.publish(char_state_topic, "Starting", true);
                delay(1000);
                client.publish(char_onoff_topic, "ON", true);
                break;
            case 2:
                client.publish(char_state_topic, "Pellet loading", true);
                break;
            case 3:
                client.publish(char_state_topic, "Ignition", true);
                break;
            case 4:
                client.publish(char_state_topic, "Work", true);
                break;
            case 5:
                client.publish(char_state_topic, "Brazier cleaning", true);
                break;
            case 6:
                client.publish(char_state_topic, "Final cleaning", true);
                break;
            case 7:
                client.publish(char_state_topic, "Standby", true);
                break;
            case 8:
                client.publish(char_state_topic, "Pellet missing", true);
                break;
            case 9:
                client.publish(char_state_topic, "Ignition failure", true);
                break;
            case 10:
                client.publish(char_state_topic, "Alarm", true);
                break;
            }
            Serial.printf("Stove %s\n", stoveState ? "ON" : "OFF");
            break;
        case ambTempAddr:
            ambTemp = val / 2;
            client.publish(char_ambtemp_topic, String(ambTemp).c_str(), true);
            Serial.printf("T. amb. %d\n", ambTemp);
            break;
        case fumesTempAddr:
            fumesTemp = val;
            client.publish(char_fumetemp_topic, String(fumesTemp).c_str(), true);
            Serial.printf("T. fumes %d\n", fumesTemp);
            break;
        case flamePowerAddr:
            flamePower = val;
            client.publish(char_flame_topic, String(flamePower).c_str(), true);
            Serial.printf("Fire %d\n", flamePower);
            break;
        case waterTempAddr:
            waterTemp = val;
            client.publish(char_watertemp_topic, String(waterTemp).c_str(), true);
            Serial.printf("T. water %d\n", waterTemp);
            break;
        }
    }
}

void getStoveState() //Get detailed stove state
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(stoveStateAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(60);
    checkStoveReply();
}

void getAmbTemp() //Get room temperature
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(ambTempAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(60);
    checkStoveReply();
}

void getFumeTemp() //Get flue gas temperature
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(fumesTempAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(60);
    checkStoveReply();
}

void getFlamePower() //Get the flame power (0, 1, 2, 3, 4, 5)
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(flamePowerAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(60);
    checkStoveReply();
}

void getWaterTemp() //Get the temperature of the water (if "idro")
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(waterTempAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(60);
    checkStoveReply();
}

void getStates() //Calls all the get…() functions
{

    getStoveState();
    delay(100);
    getAmbTemp();
    delay(100);
    getFumeTemp();
    delay(100);
    getFlamePower();
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
    attachInterrupt(digitalPinToInterrupt(RESET_PIN), fullReset, FALLING); //We setup the reinit interrupt
    Serial.begin(115200);
    StoveSerial.begin(1200, SERIAL_MODE, RX_PIN, TX_PIN, false, 256);
    if (SPIFFS.begin()) //Mount SPIFFS
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
        ambtemp_topic += mqtt_topic;
        fumetemp_topic += mqtt_topic;
        watertemp_topic += mqtt_topic;
        flame_topic += mqtt_topic;
        state_topic += mqtt_topic;
        onoff_topic += mqtt_topic;
        in_topic += mqtt_topic;
        ambtemp_topic += "/ambtemp";
        fumetemp_topic += "/fumetemp";
        watertemp_topic += "/watertemp";
        flame_topic += "/flamepower";
        state_topic += "/state";
        onoff_topic += "/onoff";
        in_topic += "/intopic";
        ambtemp_topic.toCharArray(char_ambtemp_topic, 50);
        fumetemp_topic.toCharArray(char_fumetemp_topic, 50);
        watertemp_topic.toCharArray(char_watertemp_topic, 50);
        flame_topic.toCharArray(char_flame_topic, 50);
        state_topic.toCharArray(char_state_topic, 50);
        onoff_topic.toCharArray(char_onoff_topic, 50);
        in_topic.toCharArray(char_in_topic, 50);
        mqtt_user = userString.c_str();
        mqtt_user.trim();
        mqtt_user.toCharArray(char_mqtt_user, 50);
        mqtt_pass = passString.c_str();
        mqtt_pass.trim();
        mqtt_pass.toCharArray(char_mqtt_pass, 50);
        hydro_mode = hydroString.c_str();
        hydro_mode.trim();
        int_hydro_mode = hydro_mode.toInt();
    }
    client.setServer(char_mqtt_server, int_mqtt_port);
    client.setCallback(callback);
    client.subscribe(char_in_topic);
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
        client.subscribe(char_in_topic);
    }
    client.loop();
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