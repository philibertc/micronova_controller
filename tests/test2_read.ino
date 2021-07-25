#include <SoftwareSerial.h>
SoftwareSerial StoveSerial;

//This test prints the stove's state on serial monitor.

#define stoveStatus 0x21

//0 - OFF, 1 - Starting, 2 - Pellet loading, 3 - Ignition, 4 - Work, 5 - Brazier cleaning, 6 - Final cleaning, 7 - Standby, 8 - Pellet missing alarm, 9 - Ignition failure alarm, 10 - Alarms(to be investigated)

#define ambTemp 0x01
#define fumeTemp 0x5A
#define stovePower 0x34
uint8_t tempAmbient, tempFumes, stoveState;
char stoveRxData[2];

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
    if (rxCount == 2)
    {
        byte val = stoveRxData[1];
        byte checksum = stoveRxData[0];
        byte param = checksum - val;
        Serial.printf("Param=%01x value=%01x", param, value)
        switch (param)
        { 
        case stoveStatus:
            stoveState = val;
            Serial.printf("Stove %s\n", stoveState ? "ON" : "OFF");
            break;
        case ambTemp:
            tempAmbient = val / 2;
            Serial.printf("T. amb. %d\n", tempAmbient);
            break;
        case fumeTemp:
            tempFumes = val;
            Serial.printf("T. fumes %d\n", tempFumes);
            break;
        }
    }
}

void getStoveState()
{
    const byte readByte = 0x00;
    StoveSerial.enableTx(true);
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(stoveStatus);
    StoveSerial.enableTx(false);
    checkStoveReply();
}

void getAmbTemp()
{
    const byte readByte = 0x00;
    StoveSerial.enableTx(true);
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(ambTemp);
    StoveSerial.enableTx(false);
    checkStoveReply();
}

void getFumeTemp()
{
    const byte readByte = 0x00;
    StoveSerial.enableTx(true);
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(fumeTemp);
    StoveSerial.enableTx(false);
    checkStoveReply();
}

void getStates()
{
    getStoveState();
    delay(150);
    getAmbTemp();
    delay(150);
    getFumeTemp();
}

void setup()
{
    Serial.begin(115200);
    StoveSerial.begin(1200, SWSERIAL_8N2, D4, D4, false, 256);
    delay(2000);
    getStates();
}

void loop()
{
    delay(2000);
    getStates();
}
