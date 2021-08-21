/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include<Arduino.h>
#include<DHT.h>
#define DHT_PIN 4
#define DHTTYPE DHT11

//dabble includes and the settings 
#define CUSTOM_SETTINGS
#define INCLUDE_DATALOGGER_MODULE
#include <DabbleESP32.h>
bool isFileOpen = true;
uint8_t closeFileSignalPin=2;


DHT dht(DHT_PIN,DHTTYPE);  //object to the class DHT

void intializeFile(){
    DataLogger.createFile("humidity and temperature logger");
    DataLogger.createColumn("Humidity");
    DataLogger.createColumn("Temperature");
    DataLogger.createColumn("HeatIndex");
}


extern "C" {


    void app_main()
        {   
            initArduino(); 
            Dabble.begin("Myesp32");
            DataLogger.sendSettings(&intializeFile);
            float humidity,temp,heatIndex;
            Serial.begin(115200);
            Serial.println(F("The serial port has been intialized"));
            dht.begin();    
            delay(2000);
            while (1)
            {
               humidity =  dht.readHumidity();
               temp = dht.readTemperature();
               if(isnan(humidity)||isnan(temp)){
                   Serial.println(F("Error while reading please check"));
                   
               } // end of if

                heatIndex = dht.computeHeatIndex(temp,humidity,false);
                Serial.printf("Humidity is %.2f \n", humidity);
                Serial.printf("Temperature is %.2f \n",temp);
                Serial.printf("heat index is %.2f \n", heatIndex);
                Dabble.processInput();
                if( isFileOpen == true){
                    Serial.println("writing to the file");
                    DataLogger.send("Humidity",humidity);
                    DataLogger.send("Temperature",temp);
                    DataLogger.send("HeatIndex",heatIndex);
                }

                delay(10000);
            }
            

        }
}
