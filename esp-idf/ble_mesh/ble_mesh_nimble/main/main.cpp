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
#define pinNum1 2
#define pinNum2 22
#define pinNum3 23
#define numOfElements 3

 

class relay{
    public:
    uint8_t pin;   // gpio pin 
    relay(uint8_t gpio){ 
        // default constuctor add the gpio pin to which the relay is connnected 
        pin = gpio;
        pinMode(pin,OUTPUT);
    }
    
    void changeState(uint8_t set_state){
        if(set_state==0) {
            digitalWrite(pin,LOW);
        }

        else if (set_state == 1){
            digitalWrite(pin,HIGH);
        }
    }
};

relay led[numOfElements] = {relay(pinNum1),relay(pinNum2),relay(pinNum3)};   // creating the led in the system



// this function extract the get and set op code from the context 
void example_handle_gen_onoff_msg(esp_ble_mesh_model_t *model,  // this is the model present in our node 
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_onoff_set_t *set){

    esp_ble_mesh_gen_onoff_srv_t *srv = (esp_ble_mesh_gen_onoff_srv_t *)model->user_data;
    ESP_LOGI(bleTag,"sender address is 0x%04x", ctx->addr);
    ESP_LOGI(bleTag,"model element index %d", model->element_idx);
    switch (ctx->recv_op)
    {
    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET:{
        esp_err_t ret= esp_ble_mesh_server_model_send_msg(model, ctx,ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, sizeof(srv->state.onoff), &srv->state.onoff);  // sending the value of the state to the remote client
        ESP_LOGI(bleTag,"send message error is %s",esp_err_to_name(ret) );
        ESP_LOGI(bleTag,"value of the state in the system is 0x%02X",srv->state.onoff );
        esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,sizeof(srv->state.onoff), &srv->state.onoff, ROLE_NODE);
    }break;
    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET:
    case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK:
            {
                led[model->element_idx].changeState(set->onoff);
                srv->state.onoff= set->onoff;
                esp_err_t ret= esp_ble_mesh_server_model_send_msg(model, ctx,ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, sizeof(srv->state.onoff), &srv->state.onoff);
                ESP_LOGI(bleTag,"send message error is %s",esp_err_to_name(ret) );
                ESP_LOGI(bleTag,"ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET & ack send");
                ESP_LOGI(bleTag,"value of the state in the system is 0x%02X",srv->state.onoff );
            }

    esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,sizeof(srv->state.onoff), &srv->state.onoff, ROLE_NODE);
    default:
        break;
    }

}

// provisioning callback function 
 void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,esp_ble_mesh_prov_cb_param_t *param) {
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
       // ESP_LOGI(bleTag,"net key %d\n", param->node_prov_complete.net_key);
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
        switch (param->ctx.recv_op) {
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
                node_app_idx = param->value.state_change.mod_app_bind.app_idx;
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
            
            break;
        default:
            ESP_LOGI(bleTag, "config server callback value is  0x%04x ", param->ctx.recv_op);
            break;
        }
    }
    
}

// this the callback related to the fuctioning of the element 
 void  example_ble_mesh_generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event,esp_ble_mesh_generic_server_cb_param_t *param){
     ESP_LOGI(bleTag, "event 0x%02x, opcode 0x%04x, src 0x%04x, dst 0x%04x",
        event, param->ctx.recv_op, param->ctx.addr, param->ctx.recv_dst);

    switch (event) {
     /*   1. When get_auto_rsp is set to ESP_BLE_MESH_SERVER_AUTO_RSP, no event will be
     *    callback to the application layer when Generic Get messages are received.
     * 2. When set_auto_rsp is set to ESP_BLE_MESH_SERVER_AUTO_RSP, this event will
     *    be callback to the application layer when Generic Set/Set Unack messages
     *    are received.*/
    case ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT:

        ESP_LOGI(bleTag, "ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
            param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
             led[param->model->element_idx].changeState(param->value.state_change.onoff_set.onoff);
            ESP_LOGI(bleTag, "onoff 0x%02x", param->value.state_change.onoff_set.onoff);
            
        }        

        break;
    case ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET) {
           ESP_LOGI(bleTag, "ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET");
           esp_ble_mesh_gen_onoff_srv_t *srv = (esp_ble_mesh_gen_onoff_srv_t *)param->model->user_data;
           esp_err_t ret= esp_ble_mesh_server_model_send_msg(param->model,&param->ctx,ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, sizeof(srv->state.onoff), &srv->state.onoff);  // sending the value of the state to the remote client
            ESP_LOGI(bleTag,"send message error is %s",esp_err_to_name(ret) );
           example_handle_gen_onoff_msg(param->model, &param->ctx, NULL);
        }
        break;
    case ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
            param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
            
                
            ESP_LOGI(bleTag, "onoff 0x%02x, tid 0x%02x", param->value.set.onoff.onoff, param->value.set.onoff.tid);
            if (param->value.set.onoff.op_en) {
                ESP_LOGI(bleTag, "trans_time 0x%02x, delay 0x%02x",
                    param->value.set.onoff.trans_time, param->value.set.onoff.delay);
            }
             example_handle_gen_onoff_msg(param->model, &param->ctx, &param->value.set.onoff);
        }
        break;
    default:
        ESP_LOGE(bleTag, "Unknown Generic Server event 0x%02x", event);
        break;
    }
   

}

// Generic configuration client callback function

void example_ble_mesh_config_client_cb(esp_ble_mesh_cfg_client_cb_event_t event,esp_ble_mesh_cfg_client_cb_param_t *param){
    
    esp_ble_mesh_client_common_param_t common = {0};
    uint32_t opcode;
    uint16_t addr;
    int err;

    opcode = param->params->opcode;
    addr = param->params->ctx.addr;

    ESP_LOGI(bleTag, "%s, error_code = 0x%02x, event = 0x%02x, addr: 0x%04x, opcode: 0x%04x",
             __func__, param->error_code, event, param->params->ctx.addr, opcode);

    if (param->error_code) {
        ESP_LOGE(bleTag, "Send config client message failed, opcode 0x%04x", opcode);
        return;
    }

    switch(event){
        case ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT:
            ESP_LOGI(bleTag,"ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT");
            break;

        case ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT:
            ESP_LOGI(bleTag,"ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT");
            break;
        
        case ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT:
            ESP_LOGI(bleTag,"ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT");
            break;
        case ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT:
            ESP_LOGI(bleTag,"ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT");
            break;

        case ESP_BLE_MESH_CFG_CLIENT_EVT_MAX:
            ESP_LOGI(bleTag,"ESP_BLE_MESH_CFG_CLIENT_EVT_MAX");
            break;

    }

}

// server health callback function server model 

void node_health_server_cb (esp_ble_mesh_health_server_cb_event_t event, esp_ble_mesh_health_server_cb_param_t *param){


    switch (event)
    {
    case ESP_BLE_MESH_HEALTH_SERVER_FAULT_UPDATE_COMP_EVT:
        ESP_LOGI(bleTag,"%s: ESP_BLE_MESH_HEALTH_SERVER_FAULT_UPDATE_COMP_EVT",__func__);
        break;
    
    case ESP_BLE_MESH_HEALTH_SERVER_FAULT_CLEAR_EVT:
        ESP_LOGI(bleTag,"%s: ESP_BLE_MESH_HEALTH_SERVER_FAULT_CLEAR_EVT",__func__);
        break;

    case ESP_BLE_MESH_HEALTH_SERVER_FAULT_TEST_EVT:
        ESP_LOGI(bleTag,"%s: ESP_BLE_MESH_HEALTH_SERVER_FAULT_TEST_EVT",__func__);
        break;

    case ESP_BLE_MESH_HEALTH_SERVER_ATTENTION_ON_EVT:
        ESP_LOGI(bleTag,"%s: ESP_BLE_MESH_HEALTH_SERVER_ATTENTION_ON_EVT",__func__);
        break;

    case ESP_BLE_MESH_HEALTH_SERVER_ATTENTION_OFF_EVT:
        ESP_LOGI(bleTag,"%s: ESP_BLE_MESH_HEALTH_SERVER_ATTENTION_OFF_EVT",__func__);
        break;
    
    case ESP_BLE_MESH_HEALTH_SERVER_EVT_MAX:
        ESP_LOGI(bleTag,"%s: ESP_BLE_MESH_HEALTH_SERVER_EVT_MAX",__func__);
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
    ret = esp_ble_mesh_register_generic_server_callback(example_ble_mesh_generic_server_cb);
    ESP_LOGI(bleTag,"register generic client/server call back error code : %s", esp_err_to_name(ret));
    ret = esp_ble_mesh_register_config_server_callback(example_ble_mesh_config_server_cb);
    ESP_LOGI(bleTag,"config server error code is  : %s", esp_err_to_name(ret));
    ret=esp_ble_mesh_register_config_client_callback(example_ble_mesh_config_client_cb);
    ESP_LOGI(bleTag,"config client error code is  : %s", esp_err_to_name(ret));
    ret=esp_ble_mesh_register_health_server_callback(node_health_server_cb);
    
    mesh_intitializaton();
  }
}