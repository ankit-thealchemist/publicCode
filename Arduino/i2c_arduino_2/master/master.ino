#include <Wire.h>
#include <string>

#define ADDRESS 0X08
#define SDA_PIN_GREEN 14
#define SCL_PIN_YELLOW 12
#define I2C_Freq 10000  // maximum frequency is for the esp8266 is 100khz. Devices cannot work with incompatible frequences 

TwoWire i2c = TwoWire(0);

char t[50];
void setup()
{
  // Wi re.setPins(SDA_PIN,SCL_PIN);
  //Wire.setPins(SDA_PIN,SCL_PIN);
  i2c.begin(SDA_PIN_GREEN, SCL_PIN_YELLOW, I2C_Freq);
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
}

// for scanning i2c address
/* void Scanner ()
{
  Serial.println ();
  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;

  i2c.begin();
  for (byte i = 8; i < 120; i++)
  {
    i2c.beginTransmission (i);          // Begin I2C transmission Address (i)
    if (i2c.endTransmission () == 0)  // Receive 0 = success (ACK response) 
    {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);     // PCF8574 7 bit address
      Serial.println (")");
      count++;
    }
  }
  Serial.print ("Found ");      
  Serial.print (count, DEC);        // numbers of devices
  Serial.println (" device(s).");
} */

void loop()
{
  Serial.println("Stating....");
  memset(t, 0, 10);
  // Scanner ();
  i2c.requestFrom(ADDRESS,sizeof(float));   // sender and receiver should have the same sending and reciving bytes

  // Serial.println(i2c.available());
  int i = 0;
  while (i2c.available())
  {
    t[i] = i2c.read();
    // Serial.println(i2c.read());
    i++;
  }

  Serial.println(t);

  i2c.flush();
  Serial.println("done");
  delay(5000);
}
