/**
 * @file http_client_secure_arduino.ino
 * @author Ankit Bansal (ankit.thealchemist@gmail.com)
 * @brief 
 * @version 1.1
 * @date 2021-08-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#endif

#include <ArduinoJson.h>
#include <string>

#define APIKEY "" // provide your api key here

#define SHEET_ID "" // provide your sheetid

#define RANGE "B2:B2" //  for this example I use the the static range. Since we are requesting only the cell thats why we are using the same values


// we have to provide the server's root certificate
const char* test_root_ca= \
"-----BEGIN CERTIFICATE-----\n"\
"MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n"\
"A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n"\
"Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n"\
"MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n"\
"A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n"\
"hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n"\
"v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n"\
"eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n"\
"tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n"\
"C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n"\
"zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n"\
"mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n"\
"V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n"\
"bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n"\
"3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n"\
"J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n"\
"291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n"\
"ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n"\
"AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n"\
"TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n"\
"-----END CERTIFICATE-----\n";

// Fingerprint for URL, expires on Nov 8, 2021, needs to be updated well before this date
unsigned char fingerprint[20] = {0x5e, 0x2f, 0x09, 0xe4, 0x4f, 0x0a, 0x27, 0x50, 0xe2, 0x58, 0x69, 0x89, 0x18, 0x04, 0x2d, 0xae, 0x2a, 0x08, 0xa9, 0x4b};

#ifdef ESP32
WiFiMulti WiFiMulti;
#elif defined(ESP8266)
ESP8266WiFiMulti WiFiMulti;
#endif

std::string uri;

DynamicJsonDocument doc(200); // creating the doc variable for parsing the json object

void setup()
{

    Serial.begin(115200);
    // Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

    for (uint8_t t = 4; t > 0; t--)
    {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP("redminote4", "ankit12345");
    uri = "https://sheets.googleapis.com/v4/spreadsheets/";
    uri += SHEET_ID;
    uri += "/values/";
    uri += RANGE;
    uri += "?key=";
    uri += APIKEY;
}

void loop()
{
    // wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED))
    {

#ifdef ESP32
        WiFiClientSecure client;
#elif defined(ESP8266)

        std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
#endif
        #ifdef ESP32
        // client.setInsecure();
        client.setCACert(test_root_ca);
        #elif defined(ESP8266)
        client->setFingerprint(fingerprint);
        #endif
        // Or, if you happy to ignore the SSL certificate, then use the following line instead:
        // client->setInsecure();

        HTTPClient https;

        Serial.print("[HTTPS] begin...\n");
        #ifdef ESP32
        if(https.begin(client,uri.c_str()))
        #elif defined(ESP8266)
        if (https.begin(*client, uri.c_str()))
        #endif
        { // HTTPS

            Serial.print("[HTTPS] GET...\n");
            // start connection and send HTTP header
            int httpCode = https.GET();

            // httpCode will be negative on error
            if (httpCode > 0)
            {
                // HTTP header has been send and Server response header has been handled
                Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

                // file found at server
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                {
                    String payload = https.getString();
                    // Serial.println(payload);

                    DeserializationError err = deserializeJson(doc, payload); // deserialize the json which we are receiving
                    if (err)
                    {
                        Serial.print(F("deserializeJson() failed with code "));
                        Serial.println(err.f_str());
                    }

                    const char *values = doc["values"][0][0]; // we are receiving the 2d array as we requested the range

                    Serial.printf("value we get is %s\n", values);
                }
            }
            else
            {
                Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
            }

            https.end();
        }
        else
        {
            Serial.printf("[HTTPS] Unable to connect\n");
        }
    }

    Serial.println("Wait 10s before next round...");
    delay(10000);
}