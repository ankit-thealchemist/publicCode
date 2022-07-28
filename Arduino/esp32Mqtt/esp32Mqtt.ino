#include <Arduino.h>

#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiGeneric.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"

#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <string>
#include <SPIFFS.h>
#include <Esp.h>

#define SILENCE_ALARM_PIN 12

#define SSID_LENGTH 26
#define PASS_LENGTH 64
#define DEVICE_LENGTH 64
#define MQTT_PATH 110

char wifi_ssid[SSID_LENGTH];
char wifi_pass[PASS_LENGTH];
char device_name[DEVICE_LENGTH];
String user_email = "";
String device_path = "";

std::string mqtt_door;
std::string mqtt_light;
std::string mqtt_fan;
std::string mqtt_battery;
std::string mqtt_espStatus;
std::string mqtt_switch;
std::string mqtt_silence_alarm;

const char *ssid = "";
const char *password = "";
AsyncWebServer server(80);

const char *mqttServer = "yourSErver";
const int mqttPort = 8883;
const char *mqttUser = "username";
const char *mqttPassword = "pass";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup()
{

    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    Serial.print("MAC ID :");
    Serial.println(WiFi.macAddress());

    pinMode(SILENCE_ALARM_PIN, OUTPUT);

    device_path = String(mqttUser) + "/feeds/" + user_email + "/" + WiFi.macAddress();
    Serial.println(device_path);

   // std::string mqtt_espStatus = std::string(device_path.c_str()) + "/" + "esp_status";

    
     mqtt_door = std::string(device_path.c_str()) + "/" + "door_status";
     mqtt_light = std::string(device_path.c_str()) + "/" + "light_status";
     mqtt_fan = std::string(device_path.c_str()) + "/" + "fan_status";
     mqtt_battery = std::string(device_path.c_str()) + "/" + "battery_status";
     mqtt_espStatus = std::string(device_path.c_str()) + "/" + "esp_status";
     mqtt_switch = std::string(device_path.c_str()) + "/" + "switch_status";
     mqtt_silence_alarm = std::string(device_path.c_str()) + "/" + "silence_alarm";

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }

    Serial.println();
    Serial.println(WiFi.localIP());
    Serial.println("Connected to the WiFi network");
    espClient.setInsecure();
    mqttclient.setServer(mqttServer, mqttPort);
    mqttclient.setCallback(callback);

    while (!mqttclient.connected())
    {
        Serial.println("Connecting to MQTT...");

        if (mqttclient.connect("ESP8266Client", mqttUser, mqttPassword))
        {
            Serial.println("Connected to mqtt...");
            mqttclient.subscribe(mqtt_door.c_str());
            mqttclient.subscribe((mqtt_door + "1").c_str());
            mqttclient.subscribe(mqtt_light.c_str());
            mqttclient.subscribe(mqtt_fan.c_str());
            mqttclient.subscribe(mqtt_switch.c_str());
            mqttclient.subscribe(mqtt_espStatus.c_str());
            mqttclient.subscribe(mqtt_silence_alarm.c_str());

            mqttclient.publish((mqtt_espStatus + "/ack").c_str(), "YES, ESP is Active");
        }
        else
        {
            Serial.print("failed with state : ");
            Serial.println(mqttclient.state());
            delay(2000);
        }
    }
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        if (!mqttclient.connected())
        {
            mqttclient.connect("ESP8266Client", mqttUser, mqttPassword);
        }
        mqttclient.loop();
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    std::string received_topic(topic);
    Serial.printf("payload size is %d \n", length);
    std::string msg_payload;
    char *buf =  new char[length+1];
    memset(buf,0,length+1);
    memcpy(buf,payload,length);
    msg_payload += (char*)buf;
    delete buf;
    Serial.println(received_topic.c_str());
    received_topic = received_topic.substr(received_topic.rfind("/")+1);
    Serial.println(received_topic.c_str());
    Serial.println(msg_payload.c_str());

    if(msg_payload.compare("toggle")==0){
        digitalWrite(SILENCE_ALARM_PIN,HIGH);
        delay(5000);
        digitalWrite(SILENCE_ALARM_PIN,LOW);
    }
}
