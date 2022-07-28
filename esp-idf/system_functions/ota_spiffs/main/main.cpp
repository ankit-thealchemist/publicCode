
#include <stdio.h>
#include <esp_ota_ops.h>
#include <esp_spiffs.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/timers.h>
#include <freertos/task.h>
#include <string.h>
#include <esp_system.h>
#include <fstream>
#include <iostream>
#define FILE_NAME "/s/ota.bin"
#define TAG "OTA"

/**
 * @brief This function will initialize the spiffs
 *
 */
void initialize_spiffs(void)
{
    ESP_LOGI(TAG, "initializing spiffs");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/s",
        .partition_label = NULL,
        .max_files = 2,
        .format_if_mount_failed = true

    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    switch (err)
    {
    case ESP_OK:
        ESP_LOGI(TAG, "Sucess in initializing the spiffs");
        break;
    case ESP_FAIL:
        ESP_LOGI(TAG, "failed to mount/format the file systerm");
        break;
    case ESP_ERR_NOT_FOUND:
        ESP_LOGI(TAG, "failed to find the spiffs partition");
        break;

    default:
        ESP_LOGI(TAG, "failed to initialize the spiffs %s", esp_err_to_name(err));
        break;
    }
}

void ota_task(void *pvParameters)
{
    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle=0;
    const esp_partition_t *update_partition = NULL;

    update_partition = esp_ota_get_next_update_partition(NULL);
    assert(update_partition != NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x", update_partition->subtype, update_partition->address);

    err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
    assert(update_handle != 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        ESP_LOGW(TAG, "Deleting the task");
        vTaskDelete(NULL);
    }

    std::ifstream file;
    ESP_LOGI(TAG, "opening the file");
    file.open(FILE_NAME, std::ios::binary);
    char data[1024];
    int length = 0;
    if (file)
    {
        while (true)
        {
            file.read(data, (int)1024);
            length = file.gcount();
            ESP_LOGI(TAG,"length of the file is %d",length);
            err = esp_ota_write(update_handle, (const void *)data, length);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "ota write failed");
                esp_ota_abort(update_handle);
            }
            if (file.eof())
            {
                break;
            }
        }
        file.close();
        err = esp_ota_end(update_handle);
        
        if (err != ESP_OK)
        {
            if (err == ESP_ERR_OTA_VALIDATE_FAILED)
            {
                ESP_LOGE(TAG, "Image validation failed, image is corrupted");
            }
            else
            {
                ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
            }
        }
        ESP_LOGI(TAG, "setting the boot partision");
        err = esp_ota_set_boot_partition(update_partition);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        }
        ESP_LOGI(TAG, "Prepare to restart system!");
        esp_restart();
    }
}

extern "C" void app_main(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    esp_err_t err = esp_ota_get_state_partition(running, &ota_state);
    ESP_LOGI(TAG,"ota state is %s",esp_err_to_name(err));
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK)
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
        {
            // run diagnostic function ...
            bool diagnostic_is_ok = 1;
            if (diagnostic_is_ok)
            {
                ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");
                esp_ota_mark_app_valid_cancel_rollback();
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
            else
            {
                ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
                esp_ota_mark_app_invalid_rollback_and_reboot();
                vTaskDelay(pdMS_TO_TICKS(2000 ));

            }
        }
    }
    initialize_spiffs();

    xTaskCreate(ota_task, "ota_task",8* 1024, NULL, 5, NULL);
}
