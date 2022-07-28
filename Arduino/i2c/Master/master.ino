#include<Wire.h>


int x=0;
void setup(){
    Serial.begin(115200);
    Wire.begin();


}


void loop(){
    Serial.println("Sending data to other esp... \n");
    Wire.beginTransmission(9);
    Wire.write(x);
    Wire.endTransmission();
    x++;
    delay(2000);
}