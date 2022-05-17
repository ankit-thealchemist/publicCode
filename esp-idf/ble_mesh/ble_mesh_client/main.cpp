#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp32-hal-cpu.h>
// including the mesh related header files
/*#include <esp_ble_mesh_common_api.h>
#include <esp_ble_mesh_provisioning_api.h>
#include <esp_ble_mesh_networking_api.h>
#include <esp_ble_mesh_config_model_api.h>
#include <esp_ble_mesh_generic_model_api.h>
#include <esp_ble_mesh_defs.h>
#include <esp_ble_mesh_health_model_api.h>*/
#include "initiation.h"

#define bleTag "bleApp"

uint16_t pubadd=0x0000;

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;  // creating the mutex

// Sending the data to the server model after getting the pubication address

void vSendSignalOnOff(void *param){

     // specific msg to be send for the on off server 
    esp_ble_mesh_client_common_param_t commonParam = {0};   //common parameters for the generic client 
    esp_ble_mesh_sensor_client_get_state_t get = {0};       // get parameter 
    esp_err_t ret;
    
    while(1){
        if (pubadd) //when pubadd is  defined means not null or 0x0000
        {   
            commonParam.opcode= ESP_BLE_MESH_MODEL_OP_SENSOR_GET;  // op code for the getting the senson value  
            commonParam.model= sensor_client.model;                     // client model
            commonParam.ctx.net_idx= node_net_idx;                      // net key 
            commonParam.ctx.app_idx = node_app_idx;                     // app key for the model
            commonParam.ctx.send_ttl = 3;    
            commonParam.ctx.send_rel = false;
            commonParam.msg_timeout = 0;
            commonParam.msg_role = ROLE_NODE;
            commonParam.ctx.addr= pubadd;     // setting  the publication address 
            //sending the get message to the server which respond in the callback function of the client sensor callback function 
            ret = esp_ble_mesh_sensor_client_get_state(&commonParam, &get);   

            if(ret!= ESP_OK){
            ESP_LOGE(bleTag,"\nsend message return value is %s  \n", esp_err_to_name(ret));
            }
             
        }
        
        vTaskDelay(10000/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}


// provisioning callback function 
 void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,esp_ble_mesh_prov_cb_param_t *param) {
    ESP_LOGE(bleTag,"The value of the event in mesh provising is: %d",event);
  switch (event) {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT, err_code %d", param->node_prov_enable_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT, bearer %s",
            param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT, bearer %s",
            param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
        ESP_LOGI(bleTag,"net index %d\n", param->node_prov_complete.net_idx);
        node_net_idx=param->node_prov_complete.net_idx;    //storing the net key 
        ESP_LOGI(bleTag,"net key %d\n", param->node_prov_complete.net_key);
        ESP_LOGI(bleTag,"address %d\n", param->node_prov_complete.addr);
        ESP_LOGI(bleTag,"flags %d\n", param->node_prov_complete.flags);
        ESP_LOGI(bleTag,"IV index%d\n", param->node_prov_complete.iv_index);
        break;
    case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_RESET_EVT");
        break;
    case ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT, err_code %d", param->node_set_unprov_dev_name_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_OUTPUT_NUMBER_EVT:
        ESP_LOGI(bleTag," output number is %d ", param->node_prov_output_num.number);
        break;
    case ESP_BLE_MESH_NODE_PROV_OUTPUT_STRING_EVT:
        ESP_LOGI(bleTag," output string is %s",param->node_prov_output_str.string);
        break;

    case ESP_BLE_MESH_HEARTBEAT_MESSAGE_RECV_EVT:
        ESP_LOGI(bleTag," heartbeat hops %d feature 0x%04x",param->heartbeat_msg_recv.hops,param->heartbeat_msg_recv.feature);
        break;

    case ESP_BLE_MESH_NODE_PROV_OOB_PUB_KEY_EVT:
        ESP_LOGI(bleTag,"ESP_BLE_MESH_NODE_PROV_OOB_PUB_KEY_EVT");
        break;
    
    case ESP_BLE_MESH_NODE_PROV_SET_OOB_PUB_KEY_COMP_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_SET_OOB_PUB_KEY_COMP_EVT  error code is %d", param->node_prov_set_oob_pub_key_comp.err_code);
        break;
    
    default:
        ESP_LOGI(bleTag, "provison cb event number is %d", event);
        break;
    }

}

//configuration server callback function, it is the foundation  model layer
 void example_ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,esp_ble_mesh_cfg_server_cb_param_t *param){
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        ESP_LOGE(bleTag,"config server callback function event : %d", param->ctx.recv_op );
        switch (param->ctx.recv_op) {
            ESP_LOGE(bleTag,"The config server event is: %d", param->ctx.recv_op);
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            ESP_LOGI(bleTag, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
            ESP_LOGI(bleTag, "net_idx 0x%04x, app_idx 0x%04x",
                param->value.state_change.appkey_add.net_idx,
                param->value.state_change.appkey_add.app_idx);
            ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
            ESP_LOGI(bleTag, "opcode for the procedure is %x",param->ctx.recv_op);
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            ESP_LOGI(bleTag, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
            ESP_LOGI(bleTag, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_app_bind.element_addr,
                param->value.state_change.mod_app_bind.app_idx,
                param->value.state_change.mod_app_bind.company_id,
                param->value.state_change.mod_app_bind.model_id);
            if (param->value.state_change.mod_app_bind.company_id == 0xFFFF &&
                param->value.state_change.mod_app_bind.model_id == ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_CLI) {
                node_app_idx = param->value.state_change.mod_app_bind.app_idx;   // storing the app key 
            }
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:
            ESP_LOGI(bleTag, "ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD");
            ESP_LOGI(bleTag, "elem_addr 0x%04x, sub_addr 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_sub_add.element_addr,
                param->value.state_change.mod_sub_add.sub_addr,
                param->value.state_change.mod_sub_add.company_id,
                param->value.state_change.mod_sub_add.model_id);
            break;

        case ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET:
            ESP_LOGI(bleTag, "ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET");
            ESP_LOGI(bleTag,"publish address is 0x%04x", param->value.state_change.mod_pub_set.pub_addr);
            pubadd= param->value.state_change.mod_pub_set.pub_addr;
            break;
        default:
            ESP_LOGI(bleTag, "config server callback value is  0x%04x ", param->ctx.recv_op);
            break;
        }
    }
    
}



void ble_mesh_sensor_client_cb (esp_ble_mesh_sensor_client_cb_event_t event,
                                esp_ble_mesh_sensor_client_cb_param_t *param){
    ESP_LOGI(bleTag,"Sensor client callback function is %u  remote address 0x%04x opcode is  0x%04X", event,param->params->ctx.addr,param->params->opcode);

//checking the get statement returns any error or not 

    if (param->error_code)
    {   portENTER_CRITICAL(&mux);
        ESP_LOGE(bleTag,"\nsensor send Message send error is %d \n", param->error_code);
        portEXIT_CRITICAL(&mux);
        return;
    }
    
    switch (event)
    {
    case ESP_BLE_MESH_SENSOR_CLIENT_GET_STATE_EVT:
        ESP_LOGI(bleTag,"value of the opcode received in ger state event 0x%04X", param->params->opcode);
            switch (param->params->opcode)
            {
                ESP_LOGI(bleTag,"ESP_BLE_MESH_SENSOR_CLIENT_GET_STATE_EVT op code 0x%04X", param->params->opcode);
                case ESP_BLE_MESH_MODEL_OP_SENSOR_GET:
                    ESP_LOGI(bleTag,"ESP_BLE_MESH_MODEL_OP_SENSOR_GET OP code is 0x%04X", param->params->ctx.recv_op);
                    ESP_LOGI(bleTag,"destination address of the received message is 0x%04X",param->params->ctx.recv_dst);

                        if (param->status_cb.sensor_status.marshalled_sensor_data->len)
                            {
                                ESP_LOG_BUFFER_HEX(bleTag,param->status_cb.sensor_status.marshalled_sensor_data->data,param->status_cb.sensor_status.marshalled_sensor_data->len);
                                uint8_t *data = param->status_cb.sensor_status.marshalled_sensor_data->data;
                                uint16_t length = 0;
                                for (;length<param->status_cb.sensor_status.marshalled_sensor_data->len;){
                                    uint8_t fmt = ESP_BLE_MESH_GET_SENSOR_DATA_FORMAT(data);
                                    uint8_t data_len = ESP_BLE_MESH_GET_SENSOR_DATA_LENGTH(data,fmt);
                                    uint16_t property_id = ESP_BLE_MESH_GET_SENSOR_DATA_PROPERTY_ID(data,fmt);
                                    uint8_t mpid_len = (fmt= ESP_BLE_MESH_SENSOR_DATA_FORMAT_A ? ESP_BLE_MESH_SENSOR_DATA_FORMAT_A_MPID_LEN : ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN);
                                    ESP_LOGI(bleTag,"data format %s  data length %02X  proprety id is %04X ", 
                                    fmt ==ESP_BLE_MESH_SENSOR_DATA_FORMAT_A? "A" : "B", data_len,property_id);
                                    if (data_len!= ESP_BLE_MESH_SENSOR_DATA_ZERO_LEN)
                                    {
                                        ESP_LOG_BUFFER_HEX(bleTag,data+mpid_len,data_len+1);
                                        length += data_len+mpid_len+1;
                                    }
                                    else
                                    {
                                        length += mpid_len;
                                    }
                                    
                                    data += length;

                                }// end of the for statement 
                            }  // end of the if statement
                    break;
            
                default:
                    break;
            }
        
        break;
    case ESP_BLE_MESH_SENSOR_CLIENT_SET_STATE_EVT;
        ESP_LOGI(bleTag,"ESP_BLE_MESH_SENSOR_CLIENT_SET_STATE_EVT occured");
        break;
    
    case ESP_BLE_MESH_SENSOR_CLIENT_PUBLISH_EVT:
        ESP_LOGI(bleTag,"ESP_BLE_MESH_SENSOR_CLIENT_PUBLISH_EVT occured");
        break;
    
    case ESP_BLE_MESH_SENSOR_CLIENT_TIMEOUT_EVT:
        ESP_LOGI(bleTag,"ESP_BLE_MESH_SENSOR_CLIENT_TIMEOUT_EVT occured");
        break;
    default:
        break;
    }

}


extern "C" {
  void app_main(){

    ESP_LOGI(bleTag,"line : %d ",__LINE__);
    initArduino();
    ESP_LOGI(bleTag,"func %s : ",__func__);
    esp_err_t ret;
    ESP_LOGI(bleTag,"Verion of the esp idf is  : %s",  esp_get_idf_version());
    ret=bluetooth_init();
    ESP_LOGI(bleTag, "bluetooth initialization of nimble ");
    ble_mesh_get_dev_uuid(dev_uuid);
    ret = esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);
    ESP_LOGI(bleTag,"provisioning call back set value : %s", esp_err_to_name(ret));
    ret = esp_ble_mesh_register_sensor_client_callback(ble_mesh_sensor_client_cb);
    ESP_LOGI(bleTag,"register call back error code : %s", esp_err_to_name(ret));
    ret = esp_ble_mesh_register_config_server_callback(example_ble_mesh_config_server_cb);
    ESP_LOGI(bleTag,"config server error code is  : %s", esp_err_to_name(ret));
    mesh_intitializaton();  // starting the ble mesh 

    xTaskCreate(vSendSignalOnOff, "signal off task", 3000,NULL,3,NULL);
  }
}