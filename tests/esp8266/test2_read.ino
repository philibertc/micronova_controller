#include <SoftwareSerial.h>
SoftwareSerial StoveSerial;

#define ENABLE_RX D2

//This test prints the stove's state on serial monitor.

#define stoveStatus 0x21

//0 - OFF, 1 - Starting, 2 - Pellet loading, 3 - Ignition, 4 - Work, 5 - Brazier cleaning, 6 - Final cleaning, 7 - Standby, 8 - Pellet missing alarm, 9 - Ignition failure alarm, 10 - Alarms(to be investigated)

#define stoveStateAddr 0x21
#define ambTempAddr 0x01
#define fumesTempAddr 0x3E
#define flamePowerAddr 0x34
#define waterTempAddr 0x03
#define waterPresAddr 0x3C
uint8_t stoveState, fumesTemp, flamePower, tempSet, waterTemp;
float ambTemp, waterPres;
char stoveRxData[2]; //When the heater is sending data, it sends two bytes: a checksum and the value

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
        Serial.printf("Param=%01x value=%d ", param, val);
        switch (param)
        {
        case stoveStateAddr:
            stoveState = val;
            Serial.printf("Stove state %d\n", stoveState);
            break;
        case ambTempAddr:
            ambTemp = (float)val / 2;
            Serial.print("T. amb. ");
            Serial.println(ambTemp);
            break;
        case fumesTempAddr:
            fumesTemp = val;
            Serial.printf("T. fumes %d\n", fumesTemp);
            break;
        case flamePowerAddr:
            flamePower = val;
            Serial.printf("Fire %d\n", flamePower);
            break;
        case waterTempAddr:
            waterTemp = val;
            Serial.printf("T. water %d\n", waterTemp);
            break;
        case waterPresAddr:
            waterPres = (float)val / 10;
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
    getAmbTemp();
    getFumeTemp();
    getFlamePower();
    getWaterTemp();
    getWaterPres();
}
}

void setup()
{
    pinMode(ENABLE_RX, OUTPUT);
    digitalWrite(ENABLE_RX, HIGH); //The led of the optocoupler is off
    Serial.begin(115200);
    StoveSerial.begin(1200, SWSERIAL_8N2, D3, D4, false, 256);
    delay(2000);
    getStates();
}

void loop()
{
    delay(2000);
    getStates();
}
