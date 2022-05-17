
#include<esp_log.h>
////// bluetooth header files
#include <esp_nimble_hci.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <console/console.h>
// ble mesh def header file 
#include<esp_ble_mesh_defs.h>

#define bleTag "nimble"
// nimble intitializing 
static SemaphoreHandle_t mesh_sem;
static uint8_t own_addr_type;
void ble_store_config_init(void);
static uint8_t addr_val[6] = {0};


void ble_mesh_get_dev_uuid(uint8_t *dev_uuid)
{
    memcpy(dev_uuid + 2, addr_val, BD_ADDR_LEN);
}

static void mesh_on_reset(int reason)
{
    ESP_LOGI(bleTag, "Resetting state; reason=%d", reason);
}

static void mesh_on_sync(void)
{
    int rc;

    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        ESP_LOGI(bleTag, "error determining address type; rc=%d", rc);
        return;
    }

    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    xSemaphoreGive(mesh_sem);
}

void mesh_host_task(void *param)
{
    ESP_LOGI(bleTag, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

esp_err_t bluetooth_init(void)
{
    mesh_sem = xSemaphoreCreateBinary();
    if (mesh_sem == NULL) {
        ESP_LOGE(bleTag, "Failed to create mesh semaphore");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());

    nimble_port_init();
    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = mesh_on_reset;
    ble_hs_cfg.sync_cb = mesh_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* XXX Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(mesh_host_task);

    xSemaphoreTake(mesh_sem, portMAX_DELAY);

    return ESP_OK;
}

// nimble initializing end