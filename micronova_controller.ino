#define mqtt_server "192.168.X.X"
#define mqtt_port 1883
#define mqtt_topic "micronova"
#define mqtt_user "usrname"
#define mqtt_pass "passwd"
#define hydro_mode 0

#include <SoftwareSerial.h>
SoftwareSerial StoveSerial;
#define SERIAL_MODE SWSERIAL_8N2 //8 data bits, parity none, 2 stop bits
#define RESET_PIN D5
#define RX_PIN D3
#define TX_PIN D4
#define ENABLE_RX D2

#include <ArduinoOTA.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wm;

int deepSleep = 0;
long previousMillis;

#define pong_topic mqtt_topic "/pong"
#define state_topic mqtt_topic "/state"
#define tempset_topic mqtt_topic "/tempset"
#define onoff_topic mqtt_topic "/onoff"
#define ambtemp_topic mqtt_topic "/ambtemp"
#define fumetemp_topic mqtt_topic "/fumetemp"
#define flame_topic mqtt_topic "/flamepower"
#define watertemp_topic mqtt_topic "/watertemp"
//#define waterset_topic mqtt_topic "/waterset"
#define waterpres_topic mqtt_topic "/waterpres"
#define pwrget_topic mqtt_topic "/stovepower"
#define in_topic mqtt_topic "/intopic"

#define device_information "{\"manufacturer\": \"Philibert Cheminot\",\"identifiers\": [\"7a396f39-80d2-493b-8e8e-31a70e700bc6\"],\"model\": \"Micronova Controller\",\"name\": \"Micronova Controller\",\"sw_version\": \"1.0.0.0\"}"

//0 - OFF, 1 - Starting, 2 - Pellet loading, 3 - Ignition, 4 - Work, 5 - Brazier cleaning, 6 - Final cleaning, 7 - Standby, 8 - Pellet missing alarm, 9 - Ignition failure alarm, 10 - Alarms (to be investigated)

//Checksum: Code+Address+Value on hexadecimal calculator

const char stoveOn[4] = {0x80, 0x21, 0x01, 0xA2};
const char stoveOff[4] = {0x80, 0x21, 0x06, 0xA7};
const char forceOff[4] = {0x80, 0x21, 0x00, 0xA1};

#define stoveStateAddr 0x21
#define ambTempAddr 0x01
#define ambTempAddr 0x01
#define tempSetAddr 0x7D
#define tempGetAddr 0x9D
#define pwrSetAddr 0x7F
#define pwrGetAddr 0x9F
#define fumesTempAddr 0x3E
#define flamePowerAddr 0x34
#define waterTempAddr 0x03
//#define waterSetAddr 0x36
#define waterPresAddr 0x3C

uint8_t stoveState, tempSet, pwrSet, fumesTemp, flamePower, waterTemp /*, waterSet*/;
float ambTemp, waterPres;
char stoveRxData[2]; //When the heater is sending data, it sends two bytes: a checksum and the value
int chksum, intfromtemp;
byte hexdumpo, checksum;
String rcvTemp;
String rcvData;

void setup_wifi() //Setup WiFiManager and connect to WiFi
{
    ArduinoOTA.setHostname(mqtt_topic);
    ArduinoOTA.setPassword("micronova");
    ArduinoOTA.begin();
    WiFi.mode(WIFI_STA);
    wm.setConnectTimeout(30);
    wm.autoConnect(mqtt_topic);
}

void reconnect() //Connect to MQTT server
{
    //Loop until we're reconnected
    while (!client.connected())
    {
        Serial.println(mqtt_user);
        Serial.println(mqtt_pass);
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESPClient-";
        clientId += String(random(0xffff), HEX); //Random client ID
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
        {
            client.setBufferSize(1024);
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            //Wait 5 seconds before retrying
            delay(5000);
        }
    }
    if (client.connected())
    {
      Serial.println("Creating topics for HomeAssistant");

      String switch_topic = "homeassistant/switch/Micronova/Controller/config";
      String switch_payload = "{\"name\": \"Controller\", \"command_topic\": \"" in_topic "\", \"state_topic\": \"" onoff_topic "\", \"payload_on\": \"ON\", \"payload_off\": \"OFF\", \"state_on\": \"ON\", \"state_off\": \"OFF\", \"retain\":false, \"optimistic\": false, \"qos\": 0, \"icon\": \"mdi:fire\", \"unique_id\": \"d5668e9c-843c-4330-ae53-a5bd135a4412\", \"device\": " device_information "}";
      client.publish(switch_topic.c_str(), switch_payload.c_str(), true);

      String sensor_topic = "homeassistant/sensor/Micronova/Controller/config";
      String sensor_payload = "{\"name\": \"Controller\", \"state_topic\": \"" pong_topic "\", \"qos\": 0, \"icon\": \"mdi:power\", \"unique_id\": \"0038ec49-3921-4f1b-9fa9-71acb04052fa\",\"device\": " device_information "}";
      client.publish(sensor_topic.c_str(), sensor_payload.c_str(), true);

      String temperature_sensor_topic = "homeassistant/sensor/Micronova/Temperature/config";
      String temperature_sensor_payload = "{\"name\": \"Temperature\", \"state_topic\": \"" ambtemp_topic "\", \"qos\": 0, \"device_class\": \"temperature\", \"state_class\": \"measurement\", \"unit_of_measurement\": \"°C\", \"icon\": \"mdi:thermometer\", \"unique_id\": \"9db3245e-6ace-4d14-ac07-844cd68d245c\",\"device\": " device_information "}";
      client.publish(temperature_sensor_topic.c_str(), temperature_sensor_payload.c_str(), true);

      String fumes_temperature_sensor_topic = "homeassistant/sensor/Micronova/FumesTemperature/config";
      String fumes_temperature_sensor_payload = "{\"name\": \"Fumes Temperature\", \"state_topic\": \"" fumetemp_topic "\", \"qos\": 0, \"device_class\": \"temperature\", \"state_class\": \"measurement\", \"unit_of_measurement\": \"°C\", \"icon\": \"mdi:thermometer\", \"unique_id\": \"3c72e1cf-bc22-499e-9f85-7c7e960f9a95\",\"device\": " device_information "}";
      client.publish(fumes_temperature_sensor_topic.c_str(), fumes_temperature_sensor_payload.c_str(), true);

      String state_sensor_topic = "homeassistant/sensor/Micronova/State/config";
      String state_sensor_payload = "{\"name\": \"State\", \"state_topic\": \"" state_topic "\", \"qos\": 0, \"icon\": \"mdi:fire-alert\", \"unique_id\": \"62fe5080-7668-409b-8451-323364b42eff\",\"device\": " device_information "}";
      client.publish(state_sensor_topic.c_str(), state_sensor_payload.c_str(), true);

      String flame_power_sensor_topic = "homeassistant/sensor/Micronova/FlamePower/config";
      String flame_power_sensor_payload = "{\"name\": \"Flame Power\", \"state_topic\": \"" flame_topic "\", \"qos\": 0, \"unit_of_measurement\": \"%\", \"icon\": \"mdi:fire\", \"unique_id\": \"c3aecb86-66e2-4358-bccb-e3b620f3d28b\",\"device\": " device_information "}";
      client.publish(flame_power_sensor_topic.c_str(), flame_power_sensor_payload.c_str(), true);

      if (hydro_mode == 1)
      {
        String water_temperature_sensor_topic = "homeassistant/sensor/Micronova/WaterTemperature/config";
        String water_temperature_sensor_payload = "{\"name\": \"Water Temperature\", \"state_topic\": \"" watertemp_topic "\", \"qos\": 0, \"device_class\": \"temperature\", \"state_class\": \"measurement\", \"unit_of_measurement\": \"°C\", \"icon\": \"mdi:coolant-temperature\", \"unique_id\": \"7eb9a6b2-8e26-49f0-b75a-dd97f537d856\",\"device\": " device_information "}";
        client.publish(water_temperature_sensor_topic.c_str(), water_temperature_sensor_payload.c_str(), true);

        String water_pressure_sensor_topic = "homeassistant/sensor/Micronova/WaterPressure/config";
        String water_pressure_sensor_payload = "{\"name\": \"Water Pressure\", \"state_topic\": \"" waterpres_topic "\", \"qos\": 0, \"device_class\": \"pressure\", \"unit_of_measurement\": \"bar\", \"icon\": \"mdi:gauge\", \"unique_id\": \"2e272bb3-7b55-4867-bf1a-b8772d0ae90d\", \"device\": " device_information "}";
        client.publish(water_pressure_sensor_topic.c_str(), water_pressure_sensor_payload.c_str(), true);
      }

      Serial.println("HomeAssistant topics created.");
    }
}

void IRAM_ATTR fullReset() //Reset all the settings but without erasing the program
{
    Serial.println("Resetting…");
    wm.resetSettings();
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
            if (stoveState > 5)
            {
                StoveSerial.write(stoveOn[i]);
                delay(1);
            }
            else if (stoveState == 0)
            {
                StoveSerial.write(stoveOn[i]);
                delay(1);
            }
        }
        client.publish(onoff_topic, "ON", true);
        delay(1000);
        getStates();
    }
    else if ((char)payload[1] == 'F')
    {
        for (int i = 0; i < 4; i++)
        {
            if (stoveState < 6)
            {
                if (stoveState > 0)
                {
                    StoveSerial.write(stoveOff[i]);
                    delay(1);
                }
            }
        }
        client.publish(onoff_topic, "OFF", true);
        delay(1000);
        getStates();
    }
    else if ((char)payload[0] == '0')
    {
        for (int i = 0; i < 4; i++)
        {
            if (stoveState < 6)
            {
                if (stoveState > 0)
                {
                    StoveSerial.write(stoveOff[i]);
                    delay(1);
                }
            }
        }
        client.publish(onoff_topic, "OFF", true);
        delay(1000);
        getStates();
    }
    else if ((char)payload[0] == '1')
    {
        for (int i = 0; i < 4; i++)
        {
            if (stoveState > 5)
            {
                StoveSerial.write(stoveOn[i]);
                delay(1);
            }
            else if (stoveState == 0)
            {
                StoveSerial.write(stoveOn[i]);
                delay(1);
            }
            client.publish(onoff_topic, "ON", true);
            delay(1000);
            getStates();
        }
    }
    else if ((char)payload[0] == 'f')
    {
        if ((char)payload[1] == 'o')
        {
            for (int i = 0; i < 4; i++)
            {
                StoveSerial.write(forceOff[i]);
                delay(1);
            }
            client.publish(onoff_topic, "OFF", true);
            delay(1000);
            getStates();
        }
    }
    else if ((char)payload[0] == 'S')
    {
        deepSleep = 1;
    }
    else if ((char)payload[0] == 'W')
    {
        deepSleep = 0;
    }
    else if ((char)payload[2] == 's')
    {
        fullReset();
    }
      else if ((char)payload[0] == 'T')
    {
      rcvData="";
       for (int i = 1; i < length; i++)
      {
        rcvData+=(char)payload[i];
        
      }
     intfromtemp = rcvData.toInt()*2;
     hexdumpo=(byte)intfromtemp;
     chksum = (0xA0+0x7d+hexdumpo)-256;
     StoveSerial.write((byte)0xA0);
     delay(1);
     StoveSerial.write((byte)0x7D);
     delay(1);
     StoveSerial.write((byte)intfromtemp);
     delay(1);
     StoveSerial.write((byte)chksum);
     delay(120);
     getStates();
  }
     else if ((char)payload[0] == 'P')
    {
      rcvData="";
       for (int i = 1; i < length; i++)
      {
        rcvData+=(char)payload[i];
        
      }
      intfromtemp = rcvData.toInt();
      hexdumpo=(byte)intfromtemp;
      chksum = (0xA0+0x7F+hexdumpo)-256;
      StoveSerial.write((byte)0xA0);
     delay(1);
     StoveSerial.write((byte)0x7F);
     delay(1);
     StoveSerial.write((byte)intfromtemp);
     delay(1);
     StoveSerial.write((byte)chksum);
     delay(120);
     getStates();
     
     
   }
}

void checkStoveReply() //Works only when request is RAM
{
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
        Serial.printf("Param=%01x value=%01x ", param, val);
        switch (param)
        {
        case stoveStateAddr:
            stoveState = val;
            switch (stoveState)
            {
            case 0:
                client.publish(state_topic, "Off", true);
                delay(1000);
                client.publish(onoff_topic, "OFF", true);
                break;
            case 1:
                client.publish(state_topic, "Starting", true);
                delay(1000);
                client.publish(onoff_topic, "ON", true);
                break;
            case 2:
                client.publish(state_topic, "Pellet loading", true);
                delay(1000);
                client.publish(onoff_topic, "ON", true);
                break;
            case 3:
                client.publish(state_topic, "Ignition", true);
                delay(1000);
                client.publish(onoff_topic, "ON", true);
                break;
            case 4:
                client.publish(state_topic, "Working", true);
                delay(1000);
                client.publish(onoff_topic, "ON", true);
                break;
            case 5:
                client.publish(state_topic, "Brazier cleaning", true);
                break;
            case 6:
                client.publish(state_topic, "Final cleaning", true);
                delay(1000);
                client.publish(onoff_topic, "OFF", true);
                break;
            case 7:
                client.publish(state_topic, "Standby", true);
                delay(1000);
                client.publish(onoff_topic, "OFF", true);
                break;
            case 8:
                client.publish(state_topic, "Pellet missing", true);
                break;
            case 9:
                client.publish(state_topic, "Ignition failure", true);
                delay(1000);
                client.publish(onoff_topic, "OFF", true);
                break;
            case 10:
                client.publish(state_topic, "Alarm", true);
                break;
            }
            Serial.printf("Stove %s\n", stoveState ? "ON" : "OFF");
            break;
        case ambTempAddr:
            ambTemp = (float)val / 2;
            client.publish(ambtemp_topic, String(ambTemp).c_str(), true);
            Serial.print("T. amb. ");
            Serial.println(ambTemp);
            break;
         case tempGetAddr:
            tempSet = (float)val / 2;
            client.publish(tempset_topic, String(tempSet).c_str(), true);
            Serial.printf("T. set %d\r\n", tempSet);
            break;
        case pwrGetAddr:
            pwrSet = (float)val;
            client.publish(pwrget_topic, String(pwrSet).c_str(), true);
            Serial.printf("Pwr set %d\r\n", pwrSet);
            break;
        case fumesTempAddr:
            fumesTemp = val;
            client.publish(fumetemp_topic, String(fumesTemp).c_str(), true);
            Serial.printf("T. fumes %d\n", fumesTemp);
            break;
        case flamePowerAddr:
            if (stoveState < 6)
            {
                if (stoveState > 0)
                {
                    flamePower = map(val, 0, 16, 10, 100);
                }
            }
            else
            {
                flamePower = 0;
            }
            client.publish(flame_topic, String(flamePower).c_str(), true);
            Serial.printf("Fire %d\n", flamePower);
            break;
        case waterTempAddr:
            waterTemp = val;
            client.publish(watertemp_topic, String(waterTemp).c_str(), true);
            Serial.printf("T. water %d\n", waterTemp);
            break;
        /*case waterSetAddr:
            waterSet = val;
            client.publish(waterset_topic, String(waterSet).c_str(), true);
            Serial.printf("T. water set %d\n", waterSet);
            break;*/
        case waterPresAddr:
            waterPres = (float)val / 10;
            client.publish(waterpres_topic, String(waterPres).c_str(), true);
            Serial.print("Pressure ");
            Serial.println(waterPres);
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
    delay(80);
    checkStoveReply();
}

void getAmbTemp() //Get room temperature
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(ambTempAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(80);
    checkStoveReply();
}

void getTempSet() //Get the thermostat setting
{
    const byte readByte = 0x20;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(tempSetAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(60);
    checkStoveReply();
}
void getPwrSet() //Get the thermostat setting
{
    const byte readByte = 0x20;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(pwrSetAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(80);
    checkStoveReply();
}
void getFumeTemp() //Get flue gas temperature
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(fumesTempAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(80);
    checkStoveReply();
}

void getFlamePower() //Get the flame power (0, 1, 2, 3, 4, 5)
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(flamePowerAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(80);
    checkStoveReply();
}

void getWaterTemp() //Get the temperature of the water (if you have an hydro heater)
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(waterTempAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(80);
    checkStoveReply();
}

/* void getWaterSet() //Get the temperature of the water (if you have an hydro heater)
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(waterSetAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(80);
    checkStoveReply();
}
*/

void getWaterPres() //Get the temperature of the water (if you have an hydro heater)
{
    const byte readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(waterPresAddr);
    digitalWrite(ENABLE_RX, LOW);
    delay(80);
    checkStoveReply();
}

void getStates() //Calls all the get…() functions
{
    getStoveState();
    delay(100);
    getAmbTemp();
    delay(100);
    getTempSet();
    delay(100);
    getPwrSet();
    getFumeTemp();
    delay(100);
    getFlamePower();
    if (hydro_mode == 1)
    {
        delay(100);
        getWaterTemp();
        delay(100);
        /*getWaterSet();
        delay(100);*/
        getWaterPres();
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
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    client.subscribe(in_topic);
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
        client.subscribe(in_topic);
    }
    client.loop();
    ArduinoOTA.handle();
    unsigned long currentMillis = millis();
    if (previousMillis > currentMillis)
    {
        previousMillis = 0;
    }
    if (currentMillis - previousMillis >= 25000)
    {
        previousMillis = currentMillis;
        getStates();
        client.publish(pong_topic, "Connected");
    }
    if (deepSleep == 1)   //Does not work without hardaware modification (a cable must be connected between RST and D0)
    {
        Serial.println("Deep Sleep");
        ESP.deepSleepInstant(300e6);
    }
}
