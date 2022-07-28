/**
 * @file global_config.h
 * @author Ankit Bansal (iotdevelope@gmail.com)
 * @brief This files mention the all the global parameters used. 
 * @version 1.1
 * @date 2021-11-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/**
 * @brief Provide your ssids and passowrd
 * 
 */
const char* ssid     = "redminote4"; 
const char* password = "ankit12345";

/**
 * @brief ntp pool servers
 * 
 */
#define SERVER1 "0.in.pool.ntp.org"
#define SERVER2 "1.in.pool.ntp.org"
#define SERVER3 "2.in.pool.ntp.org"

/**
 * @brief Pin to which dht sensor connects
 * 
 */
#define DHTPIN 26

/**
 * @brief type of dht senor to connect 
 * 
 */
#define DHTTYPE DHT11 

/**
 * @brief define the time to update temeperature from sensor  and save to spiffs
 * 
 */
#define TIME_TO_READ_DATA_FROM_SENSOR_IN_SECONDS 15

/**
 * @brief define the filename for sensor data
 * 
 */
#define FILENAME "/spiffs/data.txt"

/**
 * @brief Define the timezone offset in seconds. As india has +5:30
 * 
 */
#define TIMEZONE_OFFSET 19800

/**
 * @brief Default document size in the arduino json
 * 
 */
#define DEFAULT_ARDUINO_JSON_SIZE 1024

/**
 * @brief time in seconds in which the file content will be purge. 5 hours = 18000seconds
 * 
 */
#define TIME_TO_DELETE_FILE_CONTENT_IN_SECONDS 18000

/**
 * @brief provide the server name for sending the file
 * 
 */
#define SERVER_NAME "httpbin.org"

#define PORT_NO 80


/**
 * @brief Time in seconds after which the data file will be uploaded to server 1 hour =3600 seconds
 * 
 */
#define TIME_TO_SEND_DATA_TO_SERVER_IN_SECONDS 3600