
#ifndef BLE_APP_SRV_H__
#define BLE_APP_SRV_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BLE_APP_BLE_OBSERVER_PRIO
#define BLE_APP_BLE_OBSERVER_PRIO 2
#endif

#define WRITE_ON 1
#define WRITE_OFF 0
#define READ_ON 1
#define READ_OFF 0
#define NOTIFY_ON 1
#define NOTIFY_OFF 0

/* ble kettle event handler */

typedef struct ble_app_service_s ble_app_service_t;

static uint16_t* app_internal_conn_handle;
static ble_app_service_t* app_internal_ptr_to_srv;

typedef void (*ble_app_evt_handler_t) (uint16_t conn_handle, ble_app_service_t * p_app_service, uint8_t new_state);

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

#define BLE_APP_DEF(_name)                                                                     \
static ble_app_service_t _name;                                                                \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                \
                     BLE_APP_BLE_OBSERVER_PRIO,                                                  \
                     ble_app_service_on_ble_evt, &_name) 


#define APP_BLE_UUID_BASE {0x80,0x42,0x13,0x3f,0xf5,0x86,0xd1,0xbf, \ 
                          0x65,0x48,0x55,0x8e,0x5a,0x88,0xeb,0xda}
#define APP_BLE_UUID_SERVICE 0x885a
#define APP_BLE_TEMP     0x1501
#define APP_BLE_WATER_LEVEL      0x1502
#define APP_BLE_TURN_ON_OFF      0x1503

/* events */ 

typedef enum
{
    BLE_APP_EVT_NOTIFICATION_ENABLED, /**&lt; Custom value notification enabled event. */
    BLE_APP_EVT_NOTIFICATION_DISABLED, /**&lt; Custom value notification disabled event. */
} ble_app_evt_type_t;

/* Service event */

typedef struct{
  ble_app_evt_type_t evt_type;
} ble_app_evt_t;

/**/

/* callback for invoking in main function */

typedef struct{
    ble_app_evt_handler_t evt_handler;
}ble_app_service_init_t;


/* */

/************************************ Service Interface ******************************************/

uint32_t ble_app_service_init(ble_app_service_t * p_app_service, \
                                 const ble_app_service_init_t * p_app_service_init, uint16_t * conn_handler);

uint32_t app_update_cus_value(uint16_t conn_handle, \
                              ble_gatts_char_handles_t * char_handle, uint8_t* buffer, size_t len_of_buffer);

void app_read_custom_char(uint16_t conn_handle, ble_gatts_char_handles_t * char_handle, uint8_t* buffer, size_t len_of_buffer);

static uint32_t app_custom_char_add(ble_app_service_t * p_ble_app_service, ble_gatts_char_handles_t * char_handle ,\
                                uint8_t read, uint8_t write, uint8_t notify, uint16_t UUID, uint8_t* value, size_t len_value);


void ble_app_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/************************************ Client Interface ******************************************/

void app_set_temp(uint16_t value);
void app_set_water_level(uint16_t value);
uint8_t app_read_turn_on_off();

#ifdef __cplusplus
}
#endif

#endif // BLE_APP_SRV_H__

/** @} */
