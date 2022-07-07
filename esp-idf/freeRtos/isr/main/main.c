/**
 * @file main.c
 * @author Ankit Bansal (iotdevelope@gmail.com)
 * @brief Demo for the the i2c for with the gpio
 * @version 1.1
 * @date 2022-07-06
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_intr_alloc.h>
#include <ds3231.h>
#include <string.h>

/********************************************defines***********************************************************************/

#define ON_BOARD_BUTTON GPIO_NUM_0

#define I2C_MASTER_SDA GPIO_NUM_19
#define I2C_MASTER_SCL GPIO_NUM_18

#define TAG "Demo_app"

/**
 * @brief semaphore handle to signal to read the data from the i2c
 */

SemaphoreHandle_t xGpioInterruptSemaphoreHandle = NULL;

/**
 * @brief i2c dev descriptor
 */
i2c_dev_t dev;

/**
 * @brief Irs when we receive the interrupt from gpio
 *
 * @param arg
 */
static void IRAM_ATTR gpio_isr_handler(void *arg)
{

    xSemaphoreGiveFromISR(xGpioInterruptSemaphoreHandle, pdTRUE);
}

/**
 * @brief this function will intialize the i2c sensor. Put your i2c sensor initialization code here.
 *
 */
void vInitiateI2CSensor()
{

    memset(&dev, 0, sizeof(i2c_dev_t));

    ESP_ERROR_CHECK(ds3231_init_desc(&dev, 0, I2C_MASTER_SDA, I2C_MASTER_SCL));

    // struct tm tm;

    // char now[255];
    // memset(now, 0, 255);
    // strcat(now, __DATE__); // copying the date
    // strcat(now, " ");
    // strcat(now, __TIME__); // copying the time
    // ESP_LOGI(TAG, "%s", now);

    // /**
    //  * @brief converting the time to the tm structure
    //  *
    //  */
    // strptime(now, "%b %e %Y %H:%M:%S", &tm);
    // ESP_LOGD(TAG, "setting the system time");
    // ESP_ERROR_CHECK(ds3231_set_time(&dev, &tm));

}

/**
 * @brief this function reads the data from the i2c sensor and prints it on the serial console.Replace the code with your i2c sensor code.
 *
 */
void vReadI2CSensorData()
{
    struct tm time = {0};
    float temp = 0;
    ESP_LOGI(TAG, "Reading the i2c data from the ds3231");

    if (ds3231_get_temp_float(&dev, &temp) != ESP_OK)
    {
        ESP_LOGE(TAG, "Unable to read the data from the ds3231");
    }

    if (ds3231_get_time(&dev, &time) != ESP_OK)
    {
        ESP_LOGE(TAG, "Unable to read the data from the ds3231");
    }

    ESP_LOGI(TAG, "The time is %d:%d:%d", time.tm_hour, time.tm_min, time.tm_sec);
    /**
     * @brief reading the date part
     *
     */
    ESP_LOGI(TAG, "The date is %d/%d/%d", time.tm_mday, time.tm_mon+1, time.tm_year + 1900);
    ESP_LOGI(TAG, "The temperature is %f", temp);
}

/**
 * @brief handles the interrupt from the gpio and read the i2c data
 *
 * @param pvParameters
 */
void vI2C_Task(void *pvParameters)
{
    ESP_LOGI(TAG, "Created the I2C task");

    vInitiateI2CSensor();

    while (1)
    {
        xSemaphoreTake(xGpioInterruptSemaphoreHandle, portMAX_DELAY);

        vReadI2CSensorData();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

void app_main(void)
{

    /**
     * @brief creating the semaphore for interrupt handling
     *
     */
    xGpioInterruptSemaphoreHandle = xSemaphoreCreateBinary();

    gpio_set_direction(ON_BOARD_BUTTON, GPIO_MODE_INPUT);

    gpio_set_pull_mode(ON_BOARD_BUTTON, GPIO_PULLUP_ONLY);

    /**
     * @brief installling the interrupt on the gpio at positive edge
     *
     */
    gpio_set_intr_type(GPIO_NUM_0, GPIO_INTR_NEGEDGE);

    // install gpio isr service
    gpio_install_isr_service(0);
    // hook isr handler for specific gpio pin
    ESP_ERROR_CHECK(gpio_isr_handler_add(ON_BOARD_BUTTON, gpio_isr_handler, NULL));

    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreate(vI2C_Task, "vI2C_Task", 4 * 2048, NULL, 5, NULL);
}
