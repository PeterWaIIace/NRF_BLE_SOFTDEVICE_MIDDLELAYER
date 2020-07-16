

# NRF_BLE_SOFTDEVICE_MIDDLELAYER
For Nordic SDK with nRF52

This template prepare general functions to access softdevice api in abstract manner by giving set of functions: 

```
uint32_t ble_app_service_init(ble_app_service_t * p_app_service, \
                                 const ble_app_service_init_t * p_app_service_init, uint16_t * conn_handler);

uint32_t app_update_cus_value(uint16_t conn_handle, \
                              ble_gatts_char_handles_t * char_handle, uint8_t* buffer, size_t len_of_buffer);

void app_read_custom_char(uint16_t conn_handle, ble_gatts_char_handles_t * char_handle, uint8_t* buffer, size_t len_of_buffer);

static uint32_t app_custom_char_add(ble_app_service_t * p_ble_app_service, ble_gatts_char_handles_t * char_handle ,\
                                uint8_t read, uint8_t write, uint8_t notify, uint16_t UUID, uint8_t* value, size_t len_value);

```
Those functions are strongly based on Nordic Semiconductor Template.

# Adding custom characteristics

To add custom characteristics find lines 53-59 in ```ble_app_srv.h```. You can observe the APP_BLE_UUID_BASE which is main device uuid, to add characteristic simply change or edit one of the below defines with shorter uuids like ```APP_BLE_TEMP```

```
#define APP_BLE_UUID_BASE {0x80,0x42,0x13,0x3f,0xf5,0x86,0xd1,0xbf, \ 
                          0x65,0x48,0x55,0x8e,0x5a,0x88,0xeb,0xda}
#define APP_BLE_UUID_SERVICE 0x885a
#define APP_BLE_TEMP     0x1501
#define APP_BLE_WATER_LEVEL      0x1502
#define APP_BLE_TURN_ON_OFF      0x1503
```
Every characteristics has to be binded with handler, therefore next in the same file you need to add ```ble_gatts_char_handles_t``` in to struct ble_app_service_s. 

```
struct ble_app_service_s // Services
{
    uint16_t                    service_handle;
    ble_gatts_char_handles_t    temp;
    ble_gatts_char_handles_t    water_level;
    ble_gatts_char_handles_t    turn_on_off;
    uint8_t                     uuid_type;
    uint16_t                    conn_handle;
    ble_app_evt_handler_t       evt_handler;
};
```

Next in ```ble_app_srv.c``` ```ble_app_service_init``` function need to be expanded/edited, by adding/changing ```app_custom_char_add``` to inform softdevice that new characteristic should be registered. From that your characteristics should be visible after running device.

```
uint32_t ble_app_service_init(ble_app_service_t * p_app_service, \
                                 const ble_app_service_init_t * p_app_service_init, uint16_t * conn_handler){
    [...]
    
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
```

Function ```app_custom_char_add``` takes pointer to service ```p_app_service```, address to your handler ``` &(p_app_service->turn_on_off)``` , three parameters ```READ_ON, WRITE_ON, NOTIFY_OFF ``` describing characteristic behaviour, uuid ```APP_BLE_XXXXXX``` to wich handler should be binded, initial value for characteristics, and byte size of this initial value. 

# Updating custom characteristics

To update your custom characteristics you can invoke prepared function, by passing conn_handle, characteristics handle, value and size of that value;
```
uint32_t app_update_cus_value(uint16_t conn_handle, ble_gatts_char_handles_t * char_handle, uint8_t* buffer, size_t len_of_buffer)

```

Function inside is taking your informations and setting it in characteristics invoking ```sd_ble_gatts_value_set``` with ```gatts_value``` which keeps your data. 

```
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
    [...]
    
    return err_code;
    }
    
```

Example of usage can be observed below. Notice that ```app_internal_ptr_to_srv``` and ```app_internal_conn_handle``` both are defined in header file, therefore you can always access them after init (where this values are assigned).

```
void app_set_temp(uint16_t value){
  app_update_cus_value(*app_internal_conn_handle,&app_internal_ptr_to_srv->temp,(uint8_t*)(&value),sizeof(value)/sizeof(uint8_t));
}
```

# Reading custom characteristics

To update your custom characteristics you can invoke prepared function, by passing conn_handle, characteristics handle, buffer return pointer and size of that value;
```
void app_read_custom_char(uint16_t conn_handle, ble_gatts_char_handles_t * char_handle, uint8_t* buffer, size_t len_of_buffer)

```

Function is preparing static buffer ```value_buffer``` to which is filled in ```sd_ble_gatts_value_get``` and after that return buffer is filled in for according to required size.

```
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

    
```

Example of usage can be observed below. Notice that ```app_internal_ptr_to_srv``` and ```app_internal_conn_handle``` both are defined in header file, therefore you can always access them after init (where this values are assigned). 

```
uint8_t app_read_turn_on_off(){
  uint8_t value[1] = {0};
  app_read_custom_char(*app_internal_conn_handle,&app_internal_ptr_to_srv->turn_on_off,value,sizeof(value)/sizeof(uint8_t));
  return value[0];
}
```

# Usage

Function from examples can be simple invoked in main.

```
void main(){
    for (;;)
    {   

        app_set_temp(temp);
        app_set_water_level(water_level);
        on_off=app_read_turn_on_off();
        NRF_LOG_INFO("ON OFF %d",on_off);
        
        idle_state_handle();
        nrf_delay_ms(100);
    }
}
```

