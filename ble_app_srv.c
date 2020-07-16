#include "ble_app_srv.h"
#include "nrf_log.h"

/************************************ User Interface ******************************************/

void app_set_temp(uint16_t value){
  app_update_cus_value(*app_internal_conn_handle,&app_internal_ptr_to_srv->temp,(uint8_t*)(&value),sizeof(value)/sizeof(uint8_t));
}

void app_set_water_level(uint16_t value){
  app_update_cus_value(*app_internal_conn_handle,&app_internal_ptr_to_srv->water_level,(uint8_t*)(&value),sizeof(value)/sizeof(uint8_t));
}

uint8_t app_read_turn_on_off(){
  uint8_t value[1] = {0};
  app_read_custom_char(*app_internal_conn_handle,&app_internal_ptr_to_srv->turn_on_off,value,sizeof(value)/sizeof(uint8_t));
  return value[0];
}


void app_read_custom_char(uint16_t conn_handle, ble_gatts_char_handles_t * char_handle, uint8_t* buffer, size_t len_of_buffer){
     uint8_t value_buffer[len_of_buffer];
     ble_gatts_value_t ble_gatts_value_acquire = {.len = len_of_buffer,
                           .offset = 0,
                           .p_value = value_buffer};
     uint32_t err_code;
     err_code = sd_ble_gatts_value_get(conn_handle,char_handle->value_handle, &ble_gatts_value_acquire);
     if(err_code != NRF_SUCCESS){
        NRF_LOG_INFO("ERROR CODE %d",err_code);
     }
     VERIFY_SUCCESS(err_code);
     for(uint8_t i =0;i < len_of_buffer;i++){
         buffer[i] = ble_gatts_value_acquire.p_value[i];
     }
}


/*************************************** BLE LIBRARY ********************************************/

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_kettle_service   Kettle Service structure.
 * @param[in]   p_ble_evt        Event received from the BLE stack.
 */

static void on_connect(ble_app_service_t * p_app_service, ble_evt_t const * p_ble_evt)
{
    p_app_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
};

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_kettle_service   Kettle Service structure.
 * @param[in]   p_ble_evt        Event received from the BLE stack.
 */

static void on_disconnect(ble_app_service_t * p_app_service, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);

    p_app_service->conn_handle = BLE_CONN_HANDLE_INVALID;
};

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_kettle_service   Kettle Service structure.
 * @param[in]   p_ble_evt        Event received from the BLE stack.
 */


/************************************ Service Interface ******************************************/

void ble_app_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{

    ble_app_service_t * p_app = (ble_app_service_t *)p_context;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_app, p_ble_evt);
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_app, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
//            on_write(p_app, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
};


uint32_t ble_app_service_init(ble_app_service_t * p_app_service, \
                                 const ble_app_service_init_t * p_app_service_init, uint16_t * conn_handler){
    
    VERIFY_PARAM_NOT_NULL(p_app_service);
    VERIFY_PARAM_NOT_NULL(p_app_service_init);
    app_internal_conn_handle = conn_handler;
    app_internal_ptr_to_srv = p_app_service;

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_app_service->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_app_service->evt_handler = p_app_service_init->evt_handler;

    // Add service
    ble_uuid128_t base_uuid = {APP_BLE_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_app_service->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_app_service->uuid_type;
    ble_uuid.uuid = APP_BLE_UUID_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_app_service->service_handle);
    VERIFY_SUCCESS(err_code);

    // Characteristic adding
    uint8_t init_value = 0x00;
    uint32_t init_32value = 0x00;
    uint16_t init_16value = 0x00;

    err_code = app_custom_char_add(p_app_service, &(p_app_service->temp) , \
                               READ_ON,WRITE_OFF,NOTIFY_ON, APP_BLE_TEMP, &init_16value, 2);
    VERIFY_SUCCESS(err_code);     
           
    err_code = app_custom_char_add(p_app_service, &(p_app_service->water_level) , \
                               READ_ON,WRITE_OFF,NOTIFY_ON, APP_BLE_WATER_LEVEL, &init_16value, 2);
    VERIFY_SUCCESS(err_code);            
    
    err_code = app_custom_char_add(p_app_service, &(p_app_service->turn_on_off) , \
                               READ_ON,WRITE_ON,NOTIFY_OFF, APP_BLE_TURN_ON_OFF, &init_value, 1);
    VERIFY_SUCCESS(err_code);                                         
    }

uint32_t app_update_cus_value(uint16_t conn_handle, \
                              ble_gatts_char_handles_t * char_handle, uint8_t* buffer, size_t len_of_buffer){
    
    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len  = len_of_buffer;
    gatts_value.offset  = 0;
    gatts_value.p_value = buffer;

    // Update database.
 
    err_code = sd_ble_gatts_value_set(conn_handle,
                                      char_handle->value_handle,
                                      &gatts_value);
    
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

   
    if ((conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = char_handle->value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }


    return err_code;
    }

static uint32_t app_custom_char_add(ble_app_service_t * p_ble_app_service, ble_gatts_char_handles_t * char_handle ,\
                                uint8_t read, uint8_t write, uint8_t notify, uint16_t UUID, uint8_t* value, size_t len_value){
                                
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read  = read;
    char_md.char_props.write = write;
    char_md.char_props.notify = notify;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = NULL;
    char_md.p_sccd_md        = NULL;

    ble_uuid.type = p_ble_app_service->uuid_type;
    ble_uuid.uuid = UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = len_value;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = len_value;
    attr_char_value.p_value   = value;

    return sd_ble_gatts_characteristic_add(p_ble_app_service->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           char_handle);
    }
