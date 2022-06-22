/* Example of FAT filesystem on external Flash.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   This sample shows how to store files inside a FAT filesystem.
   FAT filesystem is stored in a partition inside SPI flash, using the
   flash wear levelling library.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "esp_partition.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"


const char *TAG = "example";

/**
 * @brief Definging the pin for configuration
 * 
 */
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_HOLD     21
#define PIN_WRITE_PROTECT 22
#define PIN_CS       5


// Handle of the wear levelling library instance
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

// Mount path for the partition
const char *base_path = "/NANDFS";

static esp_flash_t* initiate_flash(void);
static const esp_partition_t* add_partition(esp_flash_t* ext_flash, const char* partition_label);

static bool mount_external_flash(const char* partition_label);

void app_main(void)
{
    // Set up SPI bus and initialize the external SPI Flash chip
    esp_flash_t* flash = initiate_flash();
    if (flash == NULL) {
        return;
    }

    // Add the entire external flash chip as a partition
    const char *partition_label = "myPartition";
    add_partition(flash, partition_label);

    

    // Initialize FAT FS in the partition
    if (!mount_external_flash(partition_label)) {
        return;
    }

   

    // Create a file in FAT FS
    ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen("/NANDFS/hello.txt", "wb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "hello world I am external flash");
    fclose(f);
    ESP_LOGI(TAG, "File written");

    // Open file for reading
    ESP_LOGI(TAG, "Reading file");
    f = fopen("/NANDFS/hello.txt", "rb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[128];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);
}

/**
 * @brief Initialize the SPI flash chip
 * 
 * @return esp_flash_t* 
 */
static esp_flash_t* initiate_flash(void)
{
    const spi_bus_config_t spi_bus_config = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadhd_io_num = PIN_HOLD,
        .quadwp_io_num = PIN_WRITE_PROTECT,
    };

    const esp_flash_spi_device_config_t device_config = {
        .host_id = VSPI_HOST,
        .cs_id = 0,
        .cs_io_num = PIN_CS,
        .io_mode = SPI_FLASH_DIO,
        .speed = ESP_FLASH_40MHZ,
    };

    ESP_LOGI(TAG, "Initializing external SPI Flash");
   

    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &spi_bus_config, 1));

    // Add device to the SPI bus
    esp_flash_t* ext_flash;
    ESP_ERROR_CHECK(spi_bus_add_flash_device(&ext_flash, &device_config));

    // Probe the Flash chip and initialize it
    esp_err_t err = esp_flash_init(ext_flash);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize external Flash: %s (0x%x)", esp_err_to_name(err), err);
        return NULL;
    }

    // Print out the ID and size
    uint32_t id;
    ESP_ERROR_CHECK(esp_flash_read_id(ext_flash, &id));
    ESP_LOGI(TAG, "External Flash , size=%d KB, ID=0x%x", ext_flash->size / 1024, id);

    return ext_flash;
}


/**
 * @brief Adding the custom partition to the SPI flash chip
 * 
 * @param ext_flash 
 * @param partition_label 
 * @return const esp_partition_t* 
 */
static const esp_partition_t* add_partition(esp_flash_t* ext_flash, const char* partition_label)
{
    ESP_LOGI(TAG, "Adding external Flash as a partition, label=\"%s\", size=%d KB", partition_label, ext_flash->size / 1024);
    const esp_partition_t* fat_partition;
    ESP_ERROR_CHECK(esp_partition_register_external(ext_flash, 0, ext_flash->size, partition_label, ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, &fat_partition));
    return fat_partition;
}

/**
 * @brief mounting our partition 
 * 
 * @param partition_label 
 * @return true 
 * @return false 
 */
static bool mount_external_flash(const char* partition_label)
{
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, partition_label, &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return false;
    }
    return true;
}

