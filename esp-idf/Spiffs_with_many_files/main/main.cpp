//############################# Includes #################################################
#include<fstream>
#include<stdio.h>
#include<esp_spiffs.h>
#include<esp_log.h>
#include<esp_system.h>
//#####################  define   #######################################################
#define str(x) #x
#define expand(x) str(x)
#define Tag __FILE__ ":" expand(__LINE__)


//############################### variable declaration #####################################
void mount_spiffs(){
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
}
extern "C"
{
    void app_main()
    {
        mount_spiffs();
       
    }
}
