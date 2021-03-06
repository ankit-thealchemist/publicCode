#include <initiation.h>
#include <esp_log.h>


#define bleTag "initiation"
uint16_t node_net_idx = ESP_BLE_MESH_KEY_UNUSED;
uint16_t node_app_idx = ESP_BLE_MESH_KEY_UNUSED;
uint8_t dev_uuid[16]={0xdd,0xdd};

/* The remote node address shall be input through UART1, see board.c */

uint16_t remote_addr = ESP_BLE_MESH_ADDR_UNASSIGNED;

esp_ble_mesh_client_t config_client;

esp_ble_mesh_cfg_srv_t config_server = {
    .relay = ESP_BLE_MESH_RELAY_ENABLED,
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
    .default_ttl = 7,// default ttl value is 7 it decide the number of hops in the network 
    /* 3 transmissions with 20ms interval */
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    
};




// context for the publishing . we have 3 led so we define the 3 publishing contexting
ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_pub_0, 2 + 3, ROLE_NODE);
ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_pub_1, 2 + 3, ROLE_NODE);
ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_pub_2, 2 + 3, ROLE_NODE);

// since we have 3 led or we can called 3 elements then we need 3 user data stucture to define them in thoose 
// the state and response is set. so we have to declare the 3 stuctures

static esp_ble_mesh_gen_onoff_srv_t onoff_server_0 = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,  // responese to the client will be send automatically by the stack 
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
};

static esp_ble_mesh_gen_onoff_srv_t onoff_server_1 = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,  // responese to the client will be send automatically by the stack 
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,
};

static esp_ble_mesh_gen_onoff_srv_t onoff_server_2 = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,  // responese to the client will be send automatically by the stack 
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
};


// defining the various model and their association to the different elements 

static esp_ble_mesh_model_t root_models[] = {   // this is the root model or the model for the primary element 
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_pub_0, &onoff_server_0),  // associating the publication context and the server data for the elements
};

static esp_ble_mesh_model_t element1[] = {
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_pub_1, &onoff_server_1)

};


static esp_ble_mesh_model_t element2[] = {
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_pub_2, &onoff_server_2)

};

// the number of element in a given node relation of with the model  
static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, ESP_BLE_MESH_MODEL_NONE),// primary element
    ESP_BLE_MESH_ELEMENT(0, element1, ESP_BLE_MESH_MODEL_NONE),   
    ESP_BLE_MESH_ELEMENT(0, element2, ESP_BLE_MESH_MODEL_NONE)
};


// this is the composition of the node with the elements
esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

/* Disable OOB security for SILabs Android app */

esp_ble_mesh_prov_t provision = {
    //.static_val = static_oob_val,
    //.static_val_len = sizeof(static_oob_val),
    .uuid = dev_uuid, 
};



void mesh_intitializaton(){
    
    esp_err_t ret;
   // esp_log_level_set("*",ESP_LOG_VERBOSE);
    ret = esp_ble_mesh_init(&provision, &composition);
    if (ret) {
        ESP_LOGE(bleTag, "Initializing mesh failed (err %s)", esp_err_to_name(ret));
        return ;
    }

    ret = esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    ESP_LOGI(bleTag,"provison enable  error code : %s", esp_err_to_name(ret));
}
