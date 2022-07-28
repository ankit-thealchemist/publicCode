#include <iostream>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_spiffs.h>
#include <fstream>
#include <string>
#include <array>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"
#include <filesystem>
#include <dirent.h>

/**
 * @brief pin configuration of the SD card
 * 
 */
#define PIN_NUM_MISO 17
#define PIN_NUM_MOSI 12
#define PIN_NUM_CLK 13
#define PIN_NUM_CS 4

#define SPI_DMA_CHAN 1

#define TAG "SDCARDS"

void add_data_sd_card();

extern "C" void app_main();

void app_main()
{
    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {

        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    // place holder for the card properties. This will let us know the properties of the card type etc
    sdmmc_card_t *card;
    const char mount_point[] = "/sdcard";
    ESP_LOGI(TAG, "Initializing SD card");

    ESP_LOGI(TAG, "Using SPI peripheral");

    // sd card host based configuration
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = 19000; // maximum frequecy supported by the sd card module

    //  spi_bus_config_t bus_cfg = {
    //     .mosi_io_num = PIN_NUM_MOSI,
    //     .miso_io_num = PIN_NUM_MISO,
    //     .sclk_io_num = PIN_NUM_CLK,
    //     .quadwp_io_num = -1,
    //     .quadhd_io_num = -1,
    //     .max_transfer_sz = 4000,
    // };

    spi_bus_config_t bus_cfg;

    bus_cfg.mosi_io_num = PIN_NUM_MOSI;
    bus_cfg.miso_io_num = PIN_NUM_MISO;
    bus_cfg.sclk_io_num = PIN_NUM_CLK;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = 4000;
    bus_cfg.flags = 0;
    bus_cfg.intr_flags = 0;

    /**
     * @brief initializing the spi bus
     * 
     */
    ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = (gpio_num_t)PIN_NUM_CS;
    slot_config.host_id = (spi_host_device_t)host.slot;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. ");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    add_data_sd_card();

    struct dirent *entry;
    DIR *dp;
    dp = opendir("/sdcard");
    if (dp == NULL)
    {
        printf("Error in opening the file");
        return;
    }
    std::string path = "/sdcard/";
    std::string fullpath;

    //reading the directory content
    while ((entry = readdir(dp)))
    {
        fullpath = path + entry->d_name;
        std::cout << "Testing the file: " << entry->d_name << "\n";
        std::ifstream file(fullpath);
        if (!file)
        {
            std::cout << "File not found\n";
        }
        std::string line;
        while (getline(file, line))
        {
            std::cout << line << "\n";
        }
    }
}



void add_data_sd_card(){
    std::string path = "/sdcard/";
    std::string fullpath;
    std::string data = "Hello World";
    std::ofstream file;
    std::string filename = "test.txt";
    fullpath = path + filename;
    file.open(fullpath, std::ios::out | std::ios::app);
    file << data<<"\n";
    file.close();
}