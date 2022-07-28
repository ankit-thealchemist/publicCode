#include "Arduino.h"

#define LED 4

extern "C" void app_main()
{
    initArduino();
    pinMode(LED, OUTPUT);
    while (1)
    {
        digitalWrite(LED, HIGH);
        delay(2000);
        digitalWrite(LED, LOW);
        delay(2000);
    }
}