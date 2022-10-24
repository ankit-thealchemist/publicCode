#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <driver/ledc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "freertos/semphr.h"
#include "driver/pcnt.h"

#define LEDC_TIMER LEDC_TIMER_3
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO (4) // Define the output GPIO
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_10_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY (512)                 // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY (12345)           // Frequency in Hertz. Set frequency at 5 kHz
#define PCNT_UNIT (PCNT_UNIT_0)
#define PCNT_CHANNEL (PCNT_CHANNEL_0)
#define PCNT_GPIO_INPUT (18)

#define TAG "PWM"

void initializeLEDC()
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY, // Set output frequency at 5 kHz
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO,
        .duty = 100, // Set duty to 0%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void pcnt_initialization()
{
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = PCNT_GPIO_INPUT,
        .ctrl_gpio_num = PCNT_PIN_NOT_USED,
        .channel = PCNT_CHANNEL_0,
        .unit = PCNT_UNIT,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC, // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS, // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_KEEP, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP, // Keep the primary counter mode if high
        // // Set the maximum and minimum limit values to watch
        // .counter_h_lim = PCNT_PIN_NOT_USED,
        // .counter_l_lim = PCNT_PIN_NOT_USED,
    };

    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(PCNT_UNIT);
}

void app_main(void)
{
    ESP_LOGI(TAG, "PWM test");
    initializeLEDC();
    pcnt_initialization();
    /**
     * @brief setting the duty cycle to 50%
     */
    // ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUedTY));
    // ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
    ESP_LOGI(TAG, "resuming the pnct counter");
    // pcnt_counter_clear(PCNT_UNIT);
    /* Everything is set up, now go to counting */
    // pcnt_counter_resume(PCNT_UNIT);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint16_t pulseCount = 0;
    pcnt_counter_clear(PCNT_UNIT);
    pcnt_counter_resume(PCNT_UNIT);
    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000)); // waiting exactly for a second

        ESP_ERROR_CHECK_WITHOUT_ABORT(pcnt_get_counter_value(PCNT_UNIT, (int16_t*)&pulseCount));
        pcnt_counter_clear(PCNT_UNIT);
        ESP_LOGI(TAG, "pulseCount: %d", (uint16_t)pulseCount);
        ESP_LOGI(TAG,"Freq= %0.2fkHz",(float)pulseCount/1000);
    }
}
