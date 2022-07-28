/**
 * @file freeRtos.ino
 * @author Ankit Bansal (ankit.thealchemist@gmail.com)
 * @brief This project demonstrate the interrupt sevice Routine( ISR). In this project the pin number 19 has been configured to generate the signal 0 to 1 and 1 to 0. An interrupt has been set on gpio 4. So whenever a stable change (more than 50 ms ) has been occured on the pin it will show the interrupt on uart
 * @version 1.1
 * @date 2021-07-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freeRtos/queue.h>
#include <freertos/task.h>
#include <esp_log.h>

QueueHandle_t myQueueHandle;  // queue handle for sending the interrupt
/**
 * @brief Class for the gpio pins
 * 
 */
class gpio
{
    int gpio_number;  // stores the gpio number
    int last_state;   // stores the last state of the pin 
    unsigned long lastTime;   /// stores the last time in the isr

public:
/**
 * @brief Construct a new gpio object
 * 
 * @param number pin number
 */
    gpio(int number)
    {
        gpio_number = number;
    }
/**
 * @brief get the gpio pin number 
 * 
 * @return int  gpio number
 */
    int getgpio()
    {
        return gpio_number;
    }
    /**
     * @brief Get the Last State in the ISR
     * 
     * @return int  
     */
    int getLastState()
    {
        return last_state;
    }
    /**
     * @brief Set the Last State in ISR
     * 
     * @param state 
     */

    void setLastState(int state)
    {
        last_state = state;
    }
    /**
     * @brief Set the Gpio Number object
     * 
     * @param num 
     */
    void setGpioNumber(int num)
    {
        gpio_number = num;
    }
    /**
     * @brief Set the Last Time object
     * 
     * @param time enter the time in miliseconds
     * @return int 
     */
    int setLastTime(unsigned long time)
    {
        lastTime = time;
    }
    /**
     * @brief Get the Last Time object
     * 
     * @return unsigned long 
     */
    unsigned long getLastTime()
    {
        return lastTime;
    }
   
};

/**
 * @brief Gpio handler function
 * 
 * @param arg Takes the gpio type object
 */
void IRAM_ATTR isr(void *arg)
{
    gpio *gpio_num = (gpio *)arg;   

   
    gpio_num->setLastState(gpio_get_level((gpio_num_t)gpio_num->getgpio())); // setting the current state
    gpio_num->setLastTime(esp_timer_get_time() / 1000ULL);                   // setting the time of interrupt
    gpio_intr_disable((gpio_num_t)gpio_num->getgpio());                       //disabling the interrupt for the pin 
    xQueueSendFromISR(myQueueHandle, &gpio_num, pdFALSE); //sending the data to the queue
    portYIELD_FROM_ISR();
}


/**
 * @brief Task function where all the logic happens for the debouncing of the gpio
 * 
 * @param pvParameters 
 */
void gpioHandler(void *pvParameters)
{
    gpio *mygpio;
    while (1)
    {
        if(xQueueReceive(myQueueHandle, &mygpio, portMAX_DELAY) == pdPASS)
        {
            vTaskDelay(pdMS_TO_TICKS(51));
            int curr_state = gpio_get_level((gpio_num_t)mygpio->getgpio());

            /**
         * @brief if the current time - last time is greater than 50 ms and state of the pin donot changes then it is a sucessful transaction
         * 
         */
            if (( mygpio->getLastState() == curr_state) && (curr_state - mygpio->getLastTime())>50 )
            {

                if (curr_state == 1)
                {

                    Serial.print("changed pin is: ");
                    Serial.println(mygpio->getgpio());
                    Serial.println("pin has been chaged from 0 to 1. Please call logic here");
                }
                if(curr_state == 0)
                {

                    Serial.print("changed pin is: ");
                    Serial.println(mygpio->getgpio());
                    Serial.println("pin has been changed from 1 to 0. Please call  the logic here");
                }
            }

            
            gpio_intr_enable((gpio_num_t)mygpio->getgpio());  // enable the interrupts for the gpio pin
        }

    }

    vTaskDelete(NULL);
}

void setup()
{
    Serial.begin(115200);

    pinMode(19,OUTPUT);       // setting the pin for the testing purpose

    gpio *objGpio = new gpio(4); // creating the gpio type object on pin 4

    //setiing the interrupt pin and other configuration
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = 1ULL << objGpio->getgpio();
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = (gpio_pullup_t)1;
    gpio_config(&io_conf);
    gpio_install_isr_service(0); // install the interrupt servie 

    //hook isr handler for specific gpio pin
    gpio_isr_handler_add((gpio_num_t)(objGpio->getgpio()), isr, objGpio);
    // creating the queue
    myQueueHandle = xQueueCreate(5, sizeof(gpio *));

    if (myQueueHandle == NULL)
    {
        Serial.println("memory problem cannot create queue");
    }
    /**
     * @brief Registering the gpio handler task
     * 
     */
    xTaskCreate(gpioHandler, "gpio handler", 1000, NULL, 5, NULL);
}

void loop()
{
    // testing the gpio pin using the software type interrupt
    digitalWrite(19,HIGH);
    delay(100);
    digitalWrite(19,LOW);
    delay(100);
}