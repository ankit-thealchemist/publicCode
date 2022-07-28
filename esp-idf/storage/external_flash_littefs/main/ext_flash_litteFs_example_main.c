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
#include <esp_littlefs.h>
#include <esp_spi_flash.h>

const char *TAG = "example";

/**
 * @brief Definging the pin for configuration
 *
 */
#define PIN_NUM_MISO 26
#define PIN_NUM_MOSI 27
#define PIN_NUM_CLK 32
#define PIN_HOLD 31
#define PIN_WRITE_PROTECT 33
#define PIN_CS 25

// Handle of the wear levelling library instance
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

// Mount path for the partition
const char *base_path = "/NANDFS";

static esp_flash_t *initiate_flash(void);
static const esp_partition_t *add_partition(esp_flash_t *ext_flash, const char *partition_label);

static bool mount_external_flash(const char *partition_label);

void mountAsLitteFs(const char *partition_label);

void app_main(void)
{
    // Set up SPI bus and initialize the external SPI Flash chip
    esp_flash_t *flash = initiate_flash();
    if (flash == NULL)
    {
        return;
    }

    // Add the entire external flash chip as a partition
    const char *partition_label = "myPartition";
    add_partition(flash, partition_label);

    mountAsLitteFs(partition_label);
    

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
static esp_flash_t *initiate_flash(void)
{
    const spi_bus_config_t spi_bus_config = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
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
    esp_flash_t *ext_flash;
    ESP_ERROR_CHECK(spi_bus_add_flash_device(&ext_flash, &device_config));

    // Probe the Flash chip and initialize it
    esp_err_t err = esp_flash_init(ext_flash);
    if (err != ESP_OK)
    {
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
static const esp_partition_t *add_partition(esp_flash_t *ext_flash, const char *partition_label)
{
    ESP_LOGI(TAG, "Adding external Flash as a partition, label=\"%s\", size=%d KB", partition_label, ext_flash->size / 1024);
    const esp_partition_t *fat_partition;
    ESP_ERROR_CHECK(esp_partition_register_external(ext_flash, 0, ext_flash->size, partition_label, ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, &fat_partition));
    return fat_partition;
}


/**
 * @brief mount the partition as littlefs
 * 
 * @param partition_label 
 */
void mountAsLitteFs(const char *partition_label)
{
    ESP_LOGI(TAG, "Initializing LittelFS");
    esp_vfs_littlefs_conf_t conf = {
            .base_path = base_path,
            .partition_label = partition_label,
            .format_if_mount_failed = true,
            .dont_mount = false,
        };

        // Use settings defined above to initialize and mount LittleFS filesystem.
        // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
        esp_err_t ret = esp_vfs_littlefs_register(&conf);

        if (ret != ESP_OK)
        {
                if (ret == ESP_FAIL)
                {
                        ESP_LOGE(TAG, "Failed to mount or format filesystem");
                }
                else if (ret == ESP_ERR_NOT_FOUND)
                {
                        ESP_LOGE(TAG, "Failed to find LittleFS partition");
                }
                else
                {
                        ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
                }
                return;
        }

        size_t total = 0, used = 0;
        ret = esp_littlefs_info(conf.partition_label, &total, &used);
        if (ret != ESP_OK)
        {
                ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
        }
        else
        {
                ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
        }

}
