//############################# Includes #################################################
#include <stdio.h>
#include <esp_spiffs.h>
#include <esp_log.h>
#include <dht.h>
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/timers.h>
#include <freertos/task.h>
#include <string.h>
#include<esp_system.h>
//#####################  define   #######################################################
#define str(x) #x
#define expand(x) str(x)
#define fwl __FILE__ ":" expand(__LINE__)
#define Tag fwl
#define sensorPin 25

//############################### variable declaration ######################################

static const dht_sensor_type_t dht_type = DHT_TYPE_DHT11; // defining the dht type sensor

// data structure for the humidity and temperature
struct sensor_data
{
    int16_t temp;
    int16_t humidity;
} data;  


// this function write data into the file in every 3 seconds
void writeDataFile(TimerHandle_t timer)
{
    dht_read_data(dht_type, (gpio_num_t)sensorPin, &data.humidity, &data.temp);
    ESP_LOGI(Tag, "humidity is %d temperatureis %d", data.humidity, data.temp);
    ESP_LOGI(Tag, "initializing spiffs");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 2,
        .format_if_mount_failed = true

    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    switch (err)
    {
    case ESP_OK:
        ESP_LOGI(Tag, "Sucess in initializing the spiffs");
        break;
    case ESP_FAIL:
        ESP_LOGI(Tag, "failed to mount/format the file systerm");
        break;
    case ESP_ERR_NOT_FOUND:
        ESP_LOGI(Tag, "failed to find the spiffs partition");
        break;

    default:
        ESP_LOGI(Tag, "failed to initialize the spiffs %s", esp_err_to_name(err));
        break;
    }
    size_t total = 0, used = 0;
    err = esp_spiffs_info(conf.partition_label, &total, &used);
    if (err != ESP_OK)
    {
        ESP_LOGE(Tag, "failed to get the spiffs infomation  err code is %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(Tag, "Total space is %d  and used space is %d", total, used);
    }

    ESP_LOGI(Tag, "opening the storage");
    FILE *fp = fopen("/spiffs/data.dat", "a+");
    assert(fp);
    ESP_LOGI(Tag, "writing to file");
    fwrite(&data, sizeof(sensor_data), 1, fp);
    fclose(fp);
    esp_vfs_spiffs_unregister(conf.partition_label);
}

// this function read the data from the file and reset the file in 10 seconds
void readData(TimerHandle_t xtimer)
{   ESP_LOGI(Tag, "initializing spiffs");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 2,
        .format_if_mount_failed = true

    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    switch (err)
    {
    case ESP_OK:
        ESP_LOGI(Tag, "Sucess in initializing the spiffs");
        break;
    case ESP_FAIL:
        ESP_LOGI(Tag, "failed to mount/format the file systerm");
        break;
    case ESP_ERR_NOT_FOUND:
        ESP_LOGI(Tag, "failed to find the spiffs partition");
        break;

    default:
        ESP_LOGI(Tag, "failed to initialize the spiffs %s", esp_err_to_name(err));
        break;
    }
    FILE *fp;
    ESP_LOGI(Tag, "opening the storage");
    fp = fopen("/spiffs/data.dat", "r+");
    ESP_LOGI(Tag, "reading form file ");
    while (!feof(fp))
    {
        fread(&data, sizeof(sensor_data), 1, fp);
        ESP_LOGI(Tag, "read humidity is %d  temp is %d", data.humidity, data.temp);
    }
    data.humidity=0;
    data.temp=0;
    fclose(fp);
    fp =  fopen("/spiffs/data.dat", "w");
    fclose(fp);
    esp_vfs_spiffs_unregister(conf.partition_label);
    ESP_LOGI(Tag,"free heap size is %d ",esp_get_free_heap_size());
    ESP_LOGI(Tag,"minimum heap size is %d", esp_get_minimum_free_heap_size());
}

extern "C"
{
    void app_main()
    {
        // creating the timer for the respective tasks
        TimerHandle_t timer = xTimerCreate("sensor_data",
                                           pdMS_TO_TICKS(3000),
                                           pdTRUE,
                                           0,
                                           writeDataFile);
        assert(xTimerStart(timer, 0));

        timer = xTimerCreate("readData", pdMS_TO_TICKS(10000), pdTRUE, 0, readData);
        assert(xTimerStart(timer, 0));
    }
}
