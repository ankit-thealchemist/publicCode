/**
 * @file spiffs.ino
 * @author Ankit Bansal (iotdevelope@gmail.com)
 * @brief main file for the code
 * @version 1.1
 * @date 2021-11-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "FS.h"
#include "SPIFFS.h"
#include <time.h>
#include <WiFi.h>
#include <DHT.h>
#include <Ticker.h>
#include "global_config.h"
#include <ArduinoJson.h>
#include <fstream>
#include <String.h>
#include <HTTPClient.h>

/**
 * @brief IST time zone
 * 
 */
long timezone = TIMEZONE_OFFSET;
byte daysavetime = 0;

/**
 * @brief Initialize DHT sensor
 * 
 * @return DHT 
 */
DHT dht(DHTPIN, DHTTYPE);

/**
 * @brief initialize the ticker object
 * 
 */
Ticker update_sensor_data;

/**
 * @brief initialize the ticker object to delete the file contents
 * 
 */
Ticker delete_contents;

/**
 * @brief Ticker for uplaading the data to the server
 * 
 */
Ticker upload_data_to_server;

DynamicJsonDocument doc(DEFAULT_ARDUINO_JSON_SIZE);

void delete_file_contents()
{
    Serial.println("deleting the file contents");
    std::ofstream os;
    os.open(FILENAME, std::ios::trunc);
    os.close();
}

/**
 * @brief Read the content of the file and print it on the console. This function for the testint  purpose only
 * 
 */
void readFile()
{
    Serial.println("\nReading from file\n");
    std::ifstream is;
    is.open(FILENAME);
    std::string line;
    while (std::getline(is, line))
    {
        Serial.println(line.c_str());
    }

    is.close();
}

/**
 * @brief Open the file and write the data at the end of file
 * 
 * @param message message to be written
 */
void writeFile(String message)
{
    std::ofstream os;

    if (!os)
    {
        Serial.println("File cannot be opened, some error occured ");
        return;
    }
    os.open(FILENAME, std::ios::app);
    os << message.c_str();
    os.close();
    // readFile();
}

/**
 * @brief Read the sensor data from the dht sensor 
 * 
 */
void readSensorData()
{

    // Serial.println("Reading sensor data");
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)

    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    //   Check if any reads failed and exit early (to try again).
    if (isnan(t))
    {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }

    struct tm tmstruct;
    delay(2000);
    tmstruct.tm_year = 0;
    getLocalTime(&tmstruct, 5000);

    char timeStamp[200];
    snprintf(timeStamp, sizeof(timeStamp), " %d-%02d-%02d %02d:%02d:%02d", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);

    doc["temp"] = t;
    doc["timestamp"] = timeStamp;

    String output;
    serializeJson(doc, output);
    // writing to file
    writeFile(output);

    Serial.flush();
    sendFileToServer();
}


/**
 * @brief Sends File to server. Our file is text file with the json object as data.
 * 
 */
void sendFileToServer()
{

    std::ifstream is;
    is.open(FILENAME);

    // getting the size of file 
    const auto begin = is.tellg();
    is.seekg(0, std::ios::end);
    const auto end = is.tellg();
    int fsize = (end - begin);
    is.seekg(0);

    // getting content of file
    std::string content, line;
    while (std::getline(is, line))
    {
        content += line;
    }
    // Serial.println(content.c_str());
    // Serial.printf("file size is %d", fsize);
    WiFiClient client;

    String boundry("----abcd"); // boundry of the http request for multiform data 

    String body;
    body+= "--" + boundry;
    body+="\r\n";
    body+="Content-Disposition: form-data; name=\"myfile\"; filename=\"sensor.txt\"\r\n";
    body+="Content-Type: text/plain\r\n";
    body+="\r\n";
    body+= String(content.c_str());
    body+="\r\n";
    body+= "--" + boundry + "--\r\n";

    String header;
    header = "POST /post HTTP/1.1\r\n";
    header += "Host:" + String(SERVER_NAME) + "\r\n";
    header+= "User-Agent: Esp-32\r\n";
    header += "Accept: */*\r\n";
    header += "Content-Type: multipart/form-data; boundary=" + boundry;
    header +="\r\n";
    header += "Content-Length:" ;
    header += body.length();
    header +="\r\n";
    header +="\r\n";

   
    delay(20);

    if (!client.connect(SERVER_NAME, PORT_NO)) {
        Serial.println("Connection failed.");
        return;
    }
     client.print(header+body);
int maxloops = 0;

  //wait for the server's reply to become available
  while (!client.available() && maxloops < 5000)
  {
    maxloops++;
    delay(1); //delay 1 msec
  }
  String data;
  if (client.available() > 0)
  { 
     data = client.readString();
    //read back one line from the server
    String line = client.readStringUntil('\r');
    Serial.println(line);
  }
  else
  {
    Serial.println("client.available() timed out ");
  }
    Serial.print(data.c_str());
    Serial.println("Closing connection.");
    client.stop();

    
}

void setup()
{
    Serial.begin(115200);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Contacting Time Server");
    configTime(timezone, daysavetime * 3600, SERVER1, SERVER2, SERVER3);
    struct tm tmstruct;
    delay(2000);
    tmstruct.tm_year = 0;
    getLocalTime(&tmstruct, 5000);
    Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
    Serial.println("");

    if (!SPIFFS.begin(true))
    {
        Serial.println("Card Mount Failed");
        return;
    }

    /**
     * @brief initializing the dht sensor 
     * 
     */
    Serial.println("initializing the dht sensor");
    dht.begin();

    /**
     * @brief attaching the interrupt for reading the sensor data
     * 
     */
    update_sensor_data.attach(TIME_TO_READ_DATA_FROM_SENSOR_IN_SECONDS, readSensorData);

    /**
     * @brief attaching the interrupt for deletting the file contents after the specific time
     * 
     */
    delete_contents.attach(TIME_TO_DELETE_FILE_CONTENT_IN_SECONDS, delete_file_contents);

    /**
     * @brief Attaching the interrupt to send the data to server
     * 
     */
    upload_data_to_server.attach(TIME_TO_SEND_DATA_TO_SERVER_IN_SECONDS,sendFileToServer);
}

void loop()
{
}
