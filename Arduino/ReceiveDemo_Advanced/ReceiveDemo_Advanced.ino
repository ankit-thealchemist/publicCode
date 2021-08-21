/*
  Example for receiving
  
  https://github.com/sui77/rc-switch/
  
  If you want to visualize a telegram copy the raw data and 
  paste it into http://test.sui.li/oszi/
*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
unsigned long receivedValue;
void setup() {
  Serial.begin(115200);
 
  mySwitch.enableReceive(27);  // Receiver on interrupt 0 => that is pin #2
}

void loop() {
  if (mySwitch.available()) {
    receivedValue = 0;
    receivedValue= mySwitch.getReceivedValue();
    Serial.printf("%lu\n",receivedValue);
    mySwitch.resetAvailable();
  }
}
