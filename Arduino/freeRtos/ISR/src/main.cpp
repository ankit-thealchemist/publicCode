#include <Arduino.h>
#include <SPI.h>
#include <RTClib.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

/**
 * @brief gpio for on board
 *
 */
#define ON_BOARD_BUTTON GPIO_NUM_0
#define I2C_SDA 19
#define I2C_SCL 18

RTC_DS3231 rtc;

TwoWire ds3231 = TwoWire(0);

/**
 * @brief semaphore handle to signal to read the data from the i2c
 */

SemaphoreHandle_t xGpioInterruptSemaphoreHandle = NULL;

/**
 * @brief Irs when we receive the interrupt from gpio
 *
 * @param arg
 */
static void IRAM_ATTR gpio_isr_handler(void *arg)
{

  xSemaphoreGiveFromISR(xGpioInterruptSemaphoreHandle, NULL);
}

/**
 * @brief this function reads the data from the i2c sensor and prints it on the serial console.Replace the code with your i2c sensor code.
 *
 */
void vReadI2CSensorData()
{
  DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" \n");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
}

/**
 * @brief handles the interrupt from the gpio and read the i2c data
 *
 * @param pvParameters
 */
void vI2C_Task(void *pvParameters)
{
  ESP_LOGI(TAG, "Created the I2C task");

  while (1)
  {
    xSemaphoreTake(xGpioInterruptSemaphoreHandle, portMAX_DELAY);

    vReadI2CSensorData();

    // vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  /**
   * @brief initiating the i2c sensor
   * 
   */
  ds3231.begin(I2C_SDA, I2C_SCL);
  while (!rtc.begin(&ds3231))
  {
    Serial.println("Couldn't find RTC");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  Serial.println("RTC found");

  if(rtc.lostPower())
  {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  /**
   * @brief creating the semaphore for interrupt handling
   *
   */
  xGpioInterruptSemaphoreHandle = xSemaphoreCreateBinary();

  gpio_set_direction(ON_BOARD_BUTTON, GPIO_MODE_INPUT);

  gpio_set_pull_mode(ON_BOARD_BUTTON, GPIO_PULLUP_ONLY);

  /**
   * @brief installling the interrupt on the gpio at negative edge
   *
   */
  gpio_set_intr_type(GPIO_NUM_0, GPIO_INTR_NEGEDGE);

  // install gpio isr service
  gpio_install_isr_service(0);
  // hook isr handler for specific gpio pin
  ESP_ERROR_CHECK(gpio_isr_handler_add(ON_BOARD_BUTTON, gpio_isr_handler, NULL));

  xTaskCreate(vI2C_Task, "vI2C_Task", 4 * 2048, NULL, 5, NULL);
}

void loop()
{

  delay(20000);
}