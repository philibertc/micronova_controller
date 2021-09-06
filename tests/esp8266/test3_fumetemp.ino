#include <SoftwareSerial.h>
SoftwareSerial StoveSerial;

#define ENABLE_RX D2

#define fumeTemp 0x5A
uint8_t tempFumes;
char stoveRxData[2];

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
        Serial.prinln(val);
        }
    }
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

void setup()
{
    pinMode(ENABLE_RX, OUTPUT);
    digitalWrite(ENABLE_RX, HIGH); //The led of the optocoupler is off
    Serial.begin(115200);
    StoveSerial.begin(1200, SWSERIAL_8N2, D3, D4, false, 256);
}

void loop()
{
    delay(2000);
    getFumeTemp();
}
