#include <Wire.h>
#include <String.h>
#include <stdio.h>

#define ADDRESS 0X08
#define SDA_PIN_GREEN 12
#define SCL_PIN_YELLOW 14

char t[10];
void setup()
{
    // Wire.pins(SDA_PIN_GREEN,SCL_PIN_YELLOW);
    // Wire.setPins(SDA_PIN, SCL_PIN);
    Wire.begin(SDA_PIN_GREEN, SCL_PIN_YELLOW, ADDRESS); /* join i2c bus with address 8 */
    Wire.onReceive(receiveEvent);                       /* register receive event */
    Wire.onRequest(requestEvent);                       /* register request event */
    Serial.begin(115200);                               /* start serial for debug */
    Serial.println("Stating i2c");
}

void loop()
{
    delay(100);
}

// function that executes whenever data is received from master
void receiveEvent(int howMany)
{
    Serial.println("read Request");
    while (0 < Wire.available())
    {
        char c = Wire.read(); /* receive byte as a character */
        Serial.print(c);      /* print the character */
    }
    Serial.println(); /* to newline */
}

// function that executes whenever data is requested from master
void requestEvent()
{
    float a, b;
    a = float(22) / 7;

    // Wire.write("Hello NodeMCU\n"); /*send string on request */
    dtostrf(a, sizeof(float), 2, t);
    // ltoa(a,t,2);
    
    Serial.println(t);
        Wire.write(t);
}