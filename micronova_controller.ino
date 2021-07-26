#include <SoftwareSerial.h>
SoftwareSerial StoveSerial;

const char stoveOn[4] = {0x80, 0x21, 0x01, 0xA2};
const char stoveOff[4] = {0x80, 0x21, 0x06, 0xA7};

#define stoveStatus 0x21

//0 - Off, 1 - Starting, 2 - Pellet loading, 3 - Ignition, 4 - Work, 5 - Brazier cleaning, 6 - Final cleaning, 7 - Standby, 8 - Pellet missing alarm, 9 - Ignition failure alarm, 10 - Alarms (to be investigated)

//Checksum: Code+Address+Value

#define ambTemp 0x01
#define fumeTemp 0x5A
#define stovePower 0x34

uint8_t tempAmbient, tempFumes, powerStove;
uint8_t stoveState = 0, lastStoveVal = 0;
char stoveRxData[2];

void getStates()
{
    getStoveState();
    delay(200);
    getAmbTemp
    delay(200);
    getFumeTemp();
}

void getStoveState()
{
    const char readByte = 0x00;
    StoveSerial.enableTx(true);
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(stoveStatus);
    StoveSerial.enableTx(false);
    checkStoveReply();
}

void getAmbTemp()
{
    const char readByte = 0x00;
    StoveSerial.enableTx(true);
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(ambTemp);
    StoveSerial.enableTx(false);
    checkStoveReply();
}

void getFumeTemp()
{
    const char readByte = 0x00;
    StoveSerial.enableTx(true);
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(fumeTemp);
    StoveSerial.enableTx(false);
    checkStoveReply();
}

void checkStoveReply()
{
    uint8_t rxCount = 0;
    stoveRxData[0] = 0x00;
    stoveRxData[1] = 0x00;
    while (StoveSerial.available())
    {
        stoveRxData[rxCount] = StoveSerial.read();
        rxCount++;
    }
    if (rxCount == 2)
    {
        byte param = stoveRxData[0] - stoveRxData[1];
        byte val = stoveRxData[1];
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

void setup()
{
    Serial.begin(115200);
    StoveSerial.begin(1200, SWSERIAL_8N2, D4, D4, false, 256);
    delay(2000);
    getStates();
}

void loop()
{}

