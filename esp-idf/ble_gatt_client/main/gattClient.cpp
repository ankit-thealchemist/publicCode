
#include <stdio.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_bt.h>
#include <esp_gap_ble_api.h>
#include <string>
#include <esp_gattc_api.h>
// #include <esp_bt_defs.h>
#include <esp_bt_main.h>
#include <esp_bt_device.h>
#include <esp_gatt_common_api.h>
#include <string.h>

extern "C"
{
    void app_main(void);
}

#ifndef btName
#define btName "Default"
#endif

#define bleTag "GATTC_DEMO"            // TAG for the logging purpose
#define PROFILE_NUM 1                  // number of application in our device
#define PROFILE_A_APP_ID 0             // profile app id number
#define REMOTE_SERVICE_UUID 0x00FF     // service id which we want to search in the server
#define REMOTE_NOTIFY_CHAR_UUID 0xFF01 // the uuid of the characterstics to be searched

// uuid for the serveice
// static uint8_t service_uuid[16] = {
//     /* LSB <--------------------------------------------------------------------------------> MSB */
//     //first uuid, 16bit, [12],[13] is the value
//     0xfb,
//     0x34,
//     0x9b,
//     0x5f,
//     0x80,
//     0x00,
//     0x00,
//     0x80,
//     0x00,
//     0x10,
//     0x00,
//     0x00,
//     0xFF,
//     0x00,
//     0x00,
//     0x00,
// };

static esp_bt_uuid_t remote_filter_service_uuid; // uuid of the service to be filtered
static esp_bt_uuid_t remote_filter_char_uuid;  // uuid of the servie ot be looked
static esp_bt_uuid_t notify_descr_uuid;
// static bool connect = false;

static const char remote_device_name[] = "bledevice"; // seeking for this device in the scanning process
static bool get_server;
static esp_gattc_char_elem_t *char_elem_result = NULL;
static esp_gattc_descr_elem_t *descr_elem_result = NULL;
TaskHandle_t handle_send_data;

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param); // prototype declaration of the gap call back function
// general profile structure
struct gattc_profile_inst
{
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t char_handle;
    esp_bd_addr_t remote_bda;
};

static gattc_profile_inst gl_profile_tab[PROFILE_NUM];

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE, // active scanning of the packet
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

//starting of the program

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{ // gap callback function

    uint8_t *adv_value = NULL;
    uint8_t adv_name_value = 0;
    uint32_t duration = 15;
    switch (event)
    {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
    { // event is initiated when the scanning parameters are set
        ESP_LOGI(bleTag, "ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT");
        //the unit of the duration is second

        esp_ble_gap_start_scanning(duration);
        break;
    }

    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
    {
        //scan start complete event to indicate scan start successfully or failed
        ESP_LOGI(bleTag, "ESP_GAP_BLE_SCAN_START_COMPLETE_EVT");
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(bleTag, "scan start failed, error status = %x", param->scan_start_cmpl.status);
            break;
        }
        ESP_LOGI(bleTag, "scan start success");

        break;
    }

    case ESP_GAP_BLE_SCAN_RESULT_EVT: //event is called to show the result of the scan. It also contain the sub events
    {
        ESP_LOGI(bleTag, "ESP_GAP_BLE_SCAN_RESULT_EVT");
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt)
        {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
        {
            ESP_LOGI(bleTag, "ESP_GAP_SEARCH_INQ_RES_EVT");
            ESP_LOG_BUFFER_HEX_LEVEL(bleTag, scan_result->scan_rst.bda, 6, ESP_LOG_INFO);
            ESP_LOGI(bleTag, "searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
            //extracting the full name from the advertising packet
            adv_value = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv, ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_value);
            std::string bleDevieName="";
            for(int i =0;i<adv_name_value;i++){
                bleDevieName+= adv_value[i];
            }
            ESP_LOGI(bleTag,"device Name is %s",bleDevieName.c_str());
            adv_value = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv, ESP_BLE_AD_TYPE_16SRV_CMPL, &adv_name_value);
            ESP_LOGI(bleTag,"size of serivde id is %d\n",adv_name_value);
            uint16_t srv_uuid;
            memcpy(&srv_uuid,adv_value,adv_name_value);
            ESP_LOGI(bleTag,"service id is %02X",srv_uuid);
            ESP_LOGI(bleTag, "value of rssi  is %i", scan_result->scan_rst.rssi);
            /* ESP_LOGI(bleTag, "complete name length is: %d", adv_name_value);
            ESP_LOG_BUFFER_CHAR_LEVEL(bleTag, adv_value, adv_name_value, ESP_LOG_ERROR); */
            ESP_LOGI(bleTag, "adv data is ");
            ESP_LOG_BUFFER_HEX_LEVEL(bleTag, scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len, ESP_LOG_INFO);
            ESP_LOGI(bleTag, "scan response data  is ");
            ESP_LOG_BUFFER_HEX_LEVEL(bleTag, &scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len], scan_result->scan_rst.scan_rsp_len, ESP_LOG_INFO);
            ESP_LOGI(bleTag, "ble addr type is %d and ble event type is %d  ", scan_result->scan_rst.ble_addr_type, scan_result->scan_rst.ble_evt_type);
            if (bleDevieName != "")
            {
                if (bleDevieName==remote_device_name)
                {
                    ESP_LOGI(bleTag, "found our device now stopping the scanning ");
                    esp_ble_gap_stop_scanning(); // stops the scanning processs
                    // now opening the connection to the server
                    // esp_ble_gattc_open(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, scan_result->scan_rst.bda, scan_result->scan_rst.ble_addr_type, true);
                }
            }
            break;
        }

        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
            ESP_LOGI(bleTag, "ESP_GAP_SEARCH_INQ_CMPL_EVT");
            ESP_LOGI(bleTag, "scan result number is %d ", scan_result->scan_rst.num_resps);
            break;

        case ESP_GAP_SEARCH_DISC_RES_EVT:
            ESP_LOGI(bleTag, " ESP_GAP_SEARCH_DISC_RES_EVT");
            break;

        case ESP_GAP_SEARCH_DISC_BLE_RES_EVT:
            ESP_LOGI(bleTag, "ESP_GAP_SEARCH_DISC_BLE_RES_EVT");
            break;

        case ESP_GAP_SEARCH_DISC_CMPL_EVT:
            ESP_LOGI(bleTag, "ESP_GAP_SEARCH_DISC_CMPL_EVT");
            break;

        case ESP_GAP_SEARCH_DI_DISC_CMPL_EVT:
            ESP_LOGI(bleTag, "ESP_GAP_SEARCH_DI_DISC_CMPL_EVT");
            break;

        case ESP_GAP_SEARCH_SEARCH_CANCEL_CMPL_EVT:
            ESP_LOGI(bleTag, "ESP_GAP_SEARCH_SEARCH_CANCEL_CMPL_EVT");
            break;
        case ESP_GAP_SEARCH_INQ_DISCARD_NUM_EVT:
            ESP_LOGI(bleTag, "ESP_GAP_SEARCH_INQ_DISCARD_NUM_EVT");
            break;

        default:
            break;
        }
    }

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
    {
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(bleTag, "scan stop failed, error status = %x", param->scan_stop_cmpl.status);
            break;
        }
        ESP_LOGI(bleTag, "stop scan successfully");
        break;
    }

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
    {
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(bleTag, "adv stop failed, error status = %x", param->adv_stop_cmpl.status);
            break;
        }
        ESP_LOGI(bleTag, "stop adv successfully");
        break;
    }

    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(bleTag, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                 param->update_conn_params.status,
                 param->update_conn_params.min_int,
                 param->update_conn_params.max_int,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;

    default:
        break;

    } // end of the main switch bracket
} // end of the gab call back function

// gattc fuction register from the fuction in main
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{

    // if event is registering the first time then
    if (event == ESP_GATTC_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        }
        else
        {
            ESP_LOGI(bleTag, "reg app failed, app_id %04x, status %d",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }

    // searching the profile according to the gattc_if and calling the respective callback funtion
    do
    {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++)
        {
            if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                gattc_if == gl_profile_tab[idx].gattc_if)
            {
                if (gl_profile_tab[idx].gattc_cb)
                {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}

// this is the profile event handler for the profile
static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{

    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
    esp_err_t ret;

    switch (event)
    {
    case ESP_GATTC_REG_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_REG_EVT");
        ret = esp_ble_gap_set_scan_params(&ble_scan_params);
        ESP_LOGI(bleTag, "scan parameters are set error code is %s ", esp_err_to_name(ret));
        break;
    case ESP_GATTC_OPEN_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_OPEN_EVT");
        ESP_LOGI(bleTag, "status of the event is %d and mtu size is %d", p_data->open.status, p_data->open.mtu);
        break;
    case ESP_GATTC_CLOSE_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_CLOSE_EVT");
        break;
    case ESP_GATTC_CFG_MTU_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_CFG_MTU_EVT");
        ESP_LOGI(bleTag, "status of event is %x  and mtu size is %d", p_data->cfg_mtu.status, p_data->cfg_mtu.mtu);
        // now searching the required service uuid in the gatt client cache
        ESP_LOGI(bleTag, "searching the required uuid on server");
        esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
        break;
    case ESP_GATTC_SEARCH_CMPL_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_SEARCH_CMPL_EVT");
        ESP_LOGI(bleTag, "Event status is %d ", p_data->search_cmpl.status);
        ESP_LOGI(bleTag, "service seach source is %d ", p_data->search_cmpl.searched_service_source);
        if (get_server)
        {
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count(gattc_if,
                                                                    p_data->search_cmpl.conn_id,
                                                                    ESP_GATT_DB_CHARACTERISTIC,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                    0,
                                                                    &count);
            ESP_LOGI(bleTag, "value of the count is %d and status is %d ", count, status);
            if (count > 0)
            {
                char_elem_result = new esp_gattc_char_elem_t[sizeof(esp_gattc_char_elem_t) * count]; // allocating the memory for char element equal to the number of element found in the cache
                if (!char_elem_result)
                {
                    ESP_LOGI(bleTag, "memory cannot be allocated for char elements ");
                }

                else
                {
                    status = esp_ble_gattc_get_char_by_uuid(gattc_if,
                                                            p_data->search_cmpl.conn_id,
                                                            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                            remote_filter_char_uuid,
                                                            char_elem_result,
                                                            &count); // getting the characterstics by uuid
                    ESP_LOGI(bleTag, "status of the get characterstic by uuid is %X and count is %d  ", status, count);
                    ESP_LOGI(bleTag, "value of the property is %d ", char_elem_result[0].properties);
                    ESP_LOGI(bleTag, "uuid for the charcterstics is %x", char_elem_result[0].uuid.uuid.uuid16);
                    if (count > 0 && (char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY))
                    {
                        gl_profile_tab[PROFILE_A_APP_ID].char_handle = char_elem_result[0].char_handle;
                        ret = esp_ble_gattc_register_for_notify(gattc_if, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, char_elem_result[0].char_handle); // unabling the notification for the given characterstics
                        ESP_LOGI(bleTag, "value of the register for the notify is %s ", esp_err_to_name(ret));
                    }
                    delete char_elem_result;
                }
            }

            else
            {
                ESP_LOGI(bleTag, "no characterstic found ");
            }
        }
        break;
    case ESP_GATTC_SEARCH_RES_EVT:
    {
        ESP_LOGI(bleTag, "ESP_GATTC_SEARCH_RES_EVT");
        ESP_LOGI(bleTag, "connection Id is %d  ", p_data->search_res.conn_id);

        if ((p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16) && (p_data->search_res.srvc_id.uuid.uuid.uuid16 == remote_filter_service_uuid.uuid.uuid16))
        {
            get_server = true;
            ESP_LOGI(bleTag, "start handle is %d and end handle is %d ", p_data->search_res.start_handle, p_data->search_res.end_handle);
            ESP_LOGI(bleTag, "instance of service id is %d and searched service id is %X ", p_data->search_res.srvc_id.inst_id,
                     p_data->search_res.srvc_id.uuid.uuid.uuid16);
            ESP_LOGI(bleTag, "is primary %d", p_data->search_res.is_primary);
            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle = p_data->search_res.end_handle;
        }
        break;
    }
    case ESP_GATTC_READ_CHAR_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_READ_CHAR_EVT");
        break;
    case ESP_GATTC_WRITE_DESCR_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_WRITE_DESCR_EVT");
        ESP_LOGI(bleTag, "value of status is %d ", p_data->write.status);
        ESP_LOGI(bleTag, "connection id is %d and handle is %d ", p_data->write.conn_id, p_data->write.handle);
        ESP_LOGI(bleTag, "value of the offset is %d ", p_data->write.offset);
        ESP_LOGI(bleTag, "now writing to the characterstic value");
        xTaskNotifyGive(handle_send_data);
        break;
    case ESP_GATTC_WRITE_CHAR_EVT:
        ESP_LOGI(bleTag, "write characterstic is sucessful if value is 0 %d", p_data->write.status);
        break;

    case ESP_GATTC_EXEC_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_EXEC_EVT");
        break;
    case ESP_GATTC_NOTIFY_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_NOTIFY_EVT");
        break;
    case ESP_GATTC_SRVC_CHG_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_SRVC_CHG_EVT");
        ret = esp_ble_gattc_cache_refresh(p_data->srvc_chg.remote_bda);
        ESP_LOGI(bleTag, "refesh error code is %s ", esp_err_to_name(ret));
        break;
    case ESP_GATTC_CONGEST_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_CONGEST_EVT");
        break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_REG_FOR_NOTIFY_EVT");
        ESP_LOGI(bleTag, "status of the event is %d ", p_data->reg_for_notify.status);
        {
            uint16_t count = 0;

            esp_gatt_status_t status = esp_ble_gattc_get_attr_count(gattc_if,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                    ESP_GATT_DB_DESCRIPTOR,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                                                    &count);
            ESP_LOGI(bleTag, "status of the attr count is %d and value of count is %d", status, count);

            if (count > 0)
            {
                descr_elem_result = new esp_gattc_descr_elem_t[sizeof(esp_gattc_descr_elem_t) * count];
                if (!descr_elem_result)
                    ESP_LOGI(bleTag, "memory cannot be allocated in the heap");
                else
                {
                    status = esp_ble_gattc_get_descr_by_char_handle(gattc_if,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                    p_data->reg_for_notify.handle,
                                                                    notify_descr_uuid,
                                                                    descr_elem_result,
                                                                    &count);
                    ESP_LOGI(bleTag, "status of the description is %d and count is %d", status, count);
                    ESP_LOGI(bleTag, "UUId of descriptor is %x", descr_elem_result[0].uuid.uuid.uuid16);

                    if (count > 0 && (descr_elem_result[0].uuid.len == ESP_UUID_LEN_16) && (descr_elem_result[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG))
                    {

                        uint16_t const notify_en = 2;
                        ret = esp_ble_gattc_write_char_descr(gattc_if,
                                                             gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                             descr_elem_result[0].handle,
                                                             sizeof(notify_en),
                                                             (uint8_t *)&notify_en,
                                                             ESP_GATT_WRITE_TYPE_RSP,
                                                             ESP_GATT_AUTH_REQ_NONE);
                        ESP_LOGI(bleTag, "value of notify en is %d and size is %d ", notify_en, sizeof(notify_en));

                        ESP_LOGI(bleTag, "status of the write characterstics descriptor is %s", esp_err_to_name(ret));
                    }
                }
                delete descr_elem_result;
            }
            else
            {
                ESP_LOGI(bleTag, "descriptor cannot be found ");
            }
        }
        break;
    case ESP_GATTC_UNREG_FOR_NOTIFY_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_UNREG_FOR_NOTIFY_EVT");
        break;
    case ESP_GATTC_CONNECT_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_CONNECT_EVT");
        ESP_LOGI(bleTag, "connection id is %d and gattc_if is %d ", p_data->connect.conn_id, gattc_if);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = p_data->connect.conn_id;
        memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(bleTag, "remote bluetooth address: ");
        ESP_LOG_BUFFER_HEX_LEVEL(bleTag, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, sizeof(esp_bd_addr_t), ESP_LOG_ERROR);
        ret = esp_ble_gattc_send_mtu_req(gattc_if, p_data->connect.conn_id); // requesting the new mtu size
        ESP_LOGI(bleTag, "mtu request send error code is %s", esp_err_to_name(ret));
        break;
    case ESP_GATTC_DISCONNECT_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_DISCONNECT_EVT");
        get_server = false;
        ESP_LOGI(bleTag, "reason for disconnection is %x ", p_data->disconnect.reason);
        break;
    case ESP_GATTC_SET_ASSOC_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_SET_ASSOC_EVT");
        break;
    case ESP_GATTC_GET_ADDR_LIST_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_GET_ADDR_LIST_EVT");
        break;
    case ESP_GATTC_QUEUE_FULL_EVT:
        ESP_LOGI(bleTag, "ESP_GATTC_QUEUE_FULL_EVT");
        break;
    default:
        break;

    } // end brace for the switch
}

// declaring the parameter function for initializing the values
void initParameters()
{
    gl_profile_tab[PROFILE_A_APP_ID].gattc_cb = gattc_profile_event_handler;
    gl_profile_tab[PROFILE_A_APP_ID].gattc_if = ESP_GATT_IF_NONE;
    remote_filter_service_uuid.len = ESP_UUID_LEN_16;
    remote_filter_service_uuid.uuid.uuid16 = REMOTE_SERVICE_UUID;
    remote_filter_char_uuid.len = ESP_UUID_LEN_16;
    remote_filter_char_uuid.uuid.uuid16 = REMOTE_NOTIFY_CHAR_UUID;
    notify_descr_uuid.len = ESP_UUID_LEN_16;
    notify_descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
}

/* void xTaskSendData(void *pvParameters)
{
    uint8_t write_char_data[2];
    write_char_data[0] = 0;
    int i = 70;
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    while (1)
    {
        write_char_data[1] = i;
        esp_err_t ret = esp_ble_gattc_write_char(gl_profile_tab[PROFILE_A_APP_ID].gattc_if,
                                                 gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                 gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                                 sizeof(write_char_data),
                                                 write_char_data,
                                                 ESP_GATT_WRITE_TYPE_RSP,
                                                 ESP_GATT_AUTH_REQ_NONE);
        ESP_LOGI(bleTag, "value of the write characterstic is %s ", esp_err_to_name(ret));
        i++;
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    vTaskDelete(NULL);
}
 */

void app_main(void)
{

    esp_err_t ret;

    /* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(bleTag, "error in ininitializing the ble controller");
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK)
    {
        ESP_LOGE(bleTag, "esp_bt_controller_enable: rc=%d %s", ret, esp_err_to_name(ret));
        return;
    }

    initParameters(); // initiating the parameters for the various purposes
    // ESP_LOGE(bleTag, "Esp frequency set to : %d", ESP.getCpuFreqMHz());
    ESP_LOGI(bleTag, "Verion of the esp idf is  : %s", esp_get_idf_version());
    /* xTaskCreate(xTaskSendData,
                "send data",
                4000,
                NULL,
                2,
                &handle_send_data); */

    ret = esp_bluedroid_init();
    ESP_LOGI(bleTag, "Value of the return value of bluedroid initialize is : %s", esp_err_to_name(ret));
    ESP_ERROR_CHECK(ret);

    ret = esp_bluedroid_enable();
    ESP_LOGI(bleTag, "Value of the return value of bluedroid enable is  : %s", esp_err_to_name(ret));
    ESP_ERROR_CHECK(ret);
    const uint8_t *add = esp_bt_dev_get_address();
    esp_bd_addr_t bdAddr;
    memcpy(bdAddr, add, sizeof(esp_bd_addr_t));
    ESP_LOGI(bleTag, "Bluetooth Address is  %X:%X:%X:%X:%X:%X ", bdAddr[0], bdAddr[1], bdAddr[2], bdAddr[3], bdAddr[4], bdAddr[5]);
    ret = esp_ble_gap_set_device_name(remote_device_name);

    ESP_LOGI(bleTag, "set the device name in the system and error code is %s", esp_err_to_name(ret));
    ret = esp_ble_gap_register_callback(esp_gap_cb);
    ESP_LOGI(bleTag, "registering the gap callback function  and status is %s ", esp_err_to_name(ret));
    ret = esp_ble_gattc_register_callback(esp_gattc_cb);
    ESP_LOGI(bleTag, "registering the gattc callback function error code is %s", esp_err_to_name(ret));

    ret = esp_ble_gattc_app_register(PROFILE_A_APP_ID); // registering the application in the client
    ESP_LOGI(bleTag, "registering the app, error code is %s ", esp_err_to_name(ret));

    ret = esp_ble_gatt_set_local_mtu(500);
    ESP_LOGI(bleTag, "local mtu event called error code is %s", esp_err_to_name(ret));
}
