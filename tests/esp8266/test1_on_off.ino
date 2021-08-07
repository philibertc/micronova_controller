#include <SoftwareSerial.h>
SoftwareSerial StoveSerial;

#define ENABLE_RX D2

//This test turns on the stove, waits 2 sec and turns off the stove.

//0 - OFF, 1 - Starting, 2 - Pellet loading, 3 - Ignition, 4 - Work, 5 - Brazier cleaning, 6 - Final cleaning, 7 - Standby, 8 - Pellet missing alarm, 9 - Ignition failure alarm, 10 - Alarms (to be investigated)

//Checksum: Code+Address+Value on hexadecimal calculator

const char stoveOn[4] = {0x80, 0x21, 0x01, 0xA2};
const char stoveOff[4] = {0x80, 0x21, 0x06, 0xA7};

void setup()
{
    pinMode(ENABLE_RX, OUTPUT);
    digitalWrite(ENABLE_RX, HIGH); //Led of the optocoupler is off
    StoveSerial.begin(1200, SWSERIAL_8N2, D3, D4, false, 256);
    for (int i = 0; i < 4; i++) {
      StoveSerial.write(stoveOn[i]);
      delay(1);
    }
    delay(2000);
    for (int i = 0; i < 4; i++) {
      StoveSerial.write(stoveOff[i]);
      delay(1);
    }
}

void loop()
{}
