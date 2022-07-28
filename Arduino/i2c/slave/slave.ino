#include<Wire.h>
#include <WireSlave.h>

int x=0;
#define SDA_PIN 21
#define SCL_PIN 22

void receiveEvent(int bytes){
    x=WireSlave.read();
    Serial.println(x);
}

void setup(){
    Serial.begin(115200);
    WireSlave.begin(SDA_PIN,SCL_PIN ,9);
    WireSlave.onReceive(receiveEvent);

}



void loop(){
    WireSlave.update();
    delay(1);
}