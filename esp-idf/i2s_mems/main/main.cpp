//####################################     INCLUDES   #############################################

#include<stdio.h>
#include<driver/i2s.h>
#include<freertos/FreeRTOS.h>
#include<freertos/task.h>
#include<esp_log.h>

//#########################  defines  ####################################
#define I2S_WS  32  //serial word select
#define I2S_SD  33  // serial data in
#define I2S_SCK 26  // serial clock
#define I2S_PORT I2S_NUM_0
#define str(x) #x
#define expand(x) str(x)
#define fwl __FILE__ ":" expand(__LINE__)
#define Tag fwl

 


// The given funciton install the i2s driver
void i2s_install(){
    esp_err_t err;  // error reporting variable
    const i2s_config_t i2s_config = {
        .mode =  i2s_mode_t(I2S_MODE_MASTER|I2S_MODE_RX),
        .sample_rate= 16000,
        .bits_per_sample= I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format= I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format= I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags=0,
        .dma_buf_count = 3,
        .dma_buf_len = 300,
        .use_apll=false,
        .tx_desc_auto_clear=false,
        .fixed_mclk = -1,
    };

   err= i2s_driver_install(I2S_PORT,&i2s_config,0,NULL);
   if (err!=ESP_OK)
   {
       ESP_LOGE(Tag,"there is some problem in installing the i2s mic");
       return;
   }
   
}

// this function set the i2s  mic pin on microcontroller
void i2s_setPin(){
    esp_err_t err;  // error reporting variable
    /*const i2s_pin_config_t pin_config = {
        I2S_SCK,I2S_WS,-1,I2S_SD
    };*/
    const i2s_pin_config_t pin_config = {
      .bck_io_num = 26,    // IIS_SCLK
      .ws_io_num = 32,     // IIS_LCLK
      .data_out_num = -1,  // IIS_DSIN
      .data_in_num = 33,   // IIS_DOUT
  };

    err = i2s_set_pin(I2S_PORT,&pin_config);
     if (err!=ESP_OK)
   {
       ESP_LOGE(Tag,"there is some problem in installing the i2s mic");
       return;
   }
   err = i2s_zero_dma_buffer(I2S_PORT);
  if (err != ESP_OK) {
    ESP_LOGE(Tag, "Error in initializing dma buffer with 0");
  }
}

// this function display the sample collected to the screen 
void pvSampleDisplay(void *pvParameters){
    const int i2s_bytes_to_read = 3200;
    size_t actualByteReaded;
    uint8_t i2s_read_buffer[i2s_bytes_to_read] = {};
    while (1)
    {
        i2s_read(I2S_PORT,(void*)i2s_read_buffer,i2s_bytes_to_read,&actualByteReaded,portMAX_DELAY);
        ESP_LOGI(Tag,"the no of bytes readed from the microphone is %d\n",actualByteReaded);
    }
    vTaskDelete(NULL);

}

extern "C"
{   
    void app_main(){
        
        i2s_install();
        i2s_setPin();
        xTaskCreatePinnedToCore(pvSampleDisplay,"tMic", 8000,NULL,3,NULL,tskNO_AFFINITY);


    }
}// end of extern "C"