#include <SoftwareSerial.h>
SoftwareSerial StoveSerial;

const char stoveOn[4] = {0x80, 0x21, 0x01, 0xA2};
const char stoveOff[4] = {0x80, 0x21, 0x06, 0xA7};

#define stoveStatus 0x21
//0 - OFF, 1 - Starting, 2 - Pellet loading, 3 - Ignition, 4 - Work, 5 - Brazier cleaning, 6 - Final cleaning, 7 - Standby, 8 - Pellet missing alarm, 9 - Ignition failure alarm, 10 - Alarms (to be investigated)
#define ambTemp 0x01
#define fumeTemp 0x5A
#define stovePower 0x34

uint8_t tempAmbient, tempFumes, powerStove;
uint8_t stoveState = 0, lastStoveVal = 0;
uint32_t replyDelay = 200;
char stoveRxData[2];

void IRAM_ATTR toggleStove_debounced()
{
    if (stoveState == 0)
    {
        Serial.println("ON");
        stoveState = 1;
        StoveSerial.enableTx(true);
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stoveOn[i]);
            delayMicroseconds(800);
        }
        StoveSerial.enableTx(false);
    }
    else if (stoveState == 1)
    {
        Serial.println("OFF");
        stoveState = 0;
        StoveSerial.enableTx(true);
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stoveOff[i]);
            delayMicroseconds(800);
        }
        StoveSerial.enableTx(false);
    }
}

void IRAM_ATTR toggleStove()
{
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 100ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 100)
    {
        toggleStove_debounced();
    }
    last_interrupt_time = interrupt_time;
}

void getStates()
{
    getStoveState();
    getFumeTemp();
    getStovePower();
}

void getStoveState()
{
    const char readByte = 0x00;
    StoveSerial.enableTx(true);
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(stoveStatus);
    StoveSerial.enableTx(false);
    delay(replyDelay);
    checkStoveReply();

    StoveSerial.enableTx(true);
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(ambTemp);
    StoveSerial.enableTx(false);
    delay(replyDelay);
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
    delay(replyDelay);
    checkStoveReply();
}

void getStovePower()
{
    const char readByte = 0x00;
    StoveSerial.enableTx(true);
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(stovePower);
    StoveSerial.enableTx(false);
    delay(replyDelay);
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
        //Serial.write(stoveRxData[rxCount]);
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
        case stovePower:
            powerStove = val;
            Serial.printf("Power %d\n", powerStove);
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
    StoveSerial.begin(1200, SWSERIAL_8N2, D1, D1, false, 256);
    //StoveSerial.enableIntTx(false);
    pinMode(D3, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(D3), toggleStove, FALLING);
}

void loop()
{
    checkStoveReply();
}
