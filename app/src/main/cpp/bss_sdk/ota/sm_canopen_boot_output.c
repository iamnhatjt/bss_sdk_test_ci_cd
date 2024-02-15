//
// Created by vnbk on 28/03/2023.
//
#include <stdio.h>
#include "sm_boot_slave.h"
#include "elapsed_timer.h"

#include "app_co_init.h"

#include "sm_ota_boot_impl.h"

#define TAG "sm_canopen_boot_output"

#define _imp(p) ((sm_boot_canopen_impl_t*)(p))

extern CO CO_DEVICE;
extern CO_OD *p_co_od;

typedef enum {
    UPGRADE_STATE_IDLE,
    UPGRADE_STATE_REQUEST_UPGRADE,
    UPGRADE_STATE_SEND_FW_INFO,
    UPGRADE_STATE_SEND_SEG,
    UPGRADE_STATE_NUMBER
}CANOPEN_UPGRADE_PROCESS_STATE;

#define CANOPEN_SEND_FW_SIZE_STEP       0
#define CANOPEN_SEND_FW_CRC_STEP        1
#define CANOPEN_SEND_FW_VERSION_STEP    2

#define CANOPEN_SEND_SEG_ADDR_STEP      0
#define CANOPEN_SEND_SEG_DATA_STEP      1
#define CANOPEN_SEND_SEG_CRC_STEP       2

#define BOOT_ST_LOADING_SERVER          2

#define PERIOD_REQUEST_UPGRADE_TIME     100

typedef struct {
    sm_boot_slave_event_callback_fn_t m_callback;
    void* m_arg;
}sm_boot_output_event_t;

typedef struct {
    sm_boot_output_if_t m_base;
    int32_t m_src_node_id;

    sm_boot_output_event_t m_events[SM_BOOT_SLAVE_EVENT_NUMBER];

    const sm_fw_signature_t* m_fw_signature;
    const sm_segment_t* m_segment;

    CANOPEN_UPGRADE_PROCESS_STATE m_state;
    uint8_t m_step;

    elapsed_timer_t m_timeout;
    uint8_t m_upgrade_request;
}sm_boot_canopen_impl_t;

static int32_t sm_boot_canopen_send_fw_version(sm_boot_output_if_t* _self){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    CO_Sub_Object fw_version_obj = {
            .p_data = &_imp(_self)->m_fw_signature->m_version,
            .attr   = ODA_SDO_RW,
            .len    = 3,
            .p_ext  = NULL
    };
    CO_SDOclient_start_download(p_sdo,
                                _imp(_self)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_FW_VERSION_SUB_INDEX,
                                &fw_version_obj, 5000);

    return 0;
}

static int32_t sm_boot_canopen_send_fw_size(sm_boot_output_if_t* _self){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    CO_Sub_Object fw_size_obj = {
            .p_data = &_imp(_self)->m_fw_signature->m_size,
            .attr   = ODA_SDO_RW,
            .len    = 4,
            .p_ext  = NULL
    };
    CO_SDOclient_start_download(p_sdo,
                                _imp(_self)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_FW_SIZE_SUB_INDEX,
                                &fw_size_obj, 5000);
    return 0;
}

static int32_t sm_boot_canopen_send_fw_crc(sm_boot_output_if_t* _self){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    CO_Sub_Object fw_crc_obj = {
            .p_data = &_imp(_self)->m_fw_signature->m_crc,
            .attr   = ODA_SDO_RW,
            .len    = 2,
            .p_ext  = NULL
    };
    CO_SDOclient_start_download(p_sdo,
                                _imp(_self)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_FW_CRC_SUB_INDEX,
                                &fw_crc_obj, 5000);
    return 0;
}
static int32_t sm_boot_canopen_send_seg_addr(sm_boot_output_if_t* _self){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    CO_Sub_Object seg_addr_obj = {
            .p_data = &_imp(_self)->m_segment->m_addr,
            .attr   = ODA_SDO_RW,
            .len    = 4,
            .p_ext  = NULL
    };
    CO_SDOclient_start_download(p_sdo,
                                _imp(_self)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_SEG_ADDR_SUB_INDEX,
                                &seg_addr_obj, 5000);

    return 0;
}
static int32_t sm_boot_canopen_send_seg_data(sm_boot_output_if_t* _self){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    CO_Sub_Object seg_data_obj = {
            .p_data = _imp(_self)->m_segment->m_data,
            .attr   = ODA_SDO_RW,
            .len    = _imp(_self)->m_segment->m_length,
            .p_ext  = NULL
    };
    CO_SDOclient_start_download(p_sdo,
                                _imp(_self)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_SEG_DATA_SUB_INDEX,
                                &seg_data_obj, 50000);

    return 0;
}
static int32_t sm_boot_canopen_send_seg_crc(sm_boot_output_if_t* _self){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    CO_Sub_Object seg_crc_obj = {
            .p_data = &_imp(_self)->m_segment->m_crc,
            .attr   = ODA_SDO_RW,
            .len    = 2,
            .p_ext  = NULL
    };
    CO_SDOclient_start_download(p_sdo,
                                _imp(_self)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_SEG_CRC_SUB_INDEX,
                                &seg_crc_obj, 5000);

    return 0;
}

static int32_t sm_boot_canopen_send_request_upgrade(sm_boot_output_if_t* _self){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    if(CO_SDO_get_status(p_sdo) == CO_SDO_RT_busy){
//      //LOG_WRN(TAG, "Can bus busy");
        return -1;
    }

    if(CO_SDO_get_status(p_sdo) == CO_SDO_RT_abort){
        //LOG_WRN(TAG, "Can bus abort, reset status");
        CO_SDO_reset_status(p_sdo);
    }

    CO_Sub_Object request_upgrade_obj = {
            .p_data = &_imp(_self)->m_upgrade_request,
            .attr   = ODA_SDO_RW,
            .len    = 1,
            .p_ext  = NULL
    };
    //LOG_DBG(TAG, "Request Upgrade fw to BP");

    CO_SDOclient_start_download(p_sdo,
                                _imp(_self)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX,
                                &request_upgrade_obj, 50);


    return 0;
}

int32_t sm_boot_canopen_process(sm_boot_output_if_t* _self){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    if(!_self || !p_sdo){
        return -1;
    }

    CO_SDO_return_t sdo_status = CO_SDO_get_status(p_sdo);

    switch (_imp(_self)->m_state) {
        case UPGRADE_STATE_IDLE:
            break;
        case UPGRADE_STATE_REQUEST_UPGRADE:
            if(!elapsed_timer_get_remain(&_imp(_self)->m_timeout)){
                sm_boot_canopen_send_request_upgrade(_self);
                elapsed_timer_reset(&_imp(_self)->m_timeout);
            }
            break;

        case UPGRADE_STATE_SEND_FW_INFO:
            if(_imp(_self)->m_step == CANOPEN_SEND_FW_SIZE_STEP && (sdo_status == CO_SDO_RT_success || sdo_status == CO_SDO_RT_idle)){
                //LOG_DBG(TAG, "Send fw SIZE");
                sm_boot_canopen_send_fw_size(_self);
                _imp(_self)->m_step++;
            }else if(_imp(_self)->m_step == CANOPEN_SEND_FW_CRC_STEP && (sdo_status == CO_SDO_RT_success || sdo_status == CO_SDO_RT_idle)){
                //LOG_DBG(TAG, "Send fw CRC");
                sm_boot_canopen_send_fw_crc(_self);
                _imp(_self)->m_step++;
            }else if(_imp(_self)->m_step == CANOPEN_SEND_FW_VERSION_STEP && (sdo_status == CO_SDO_RT_success || sdo_status == CO_SDO_RT_idle)){
                //LOG_DBG(TAG, "Send fw VERSION");
                sm_boot_canopen_send_fw_version(_self);
                _imp(_self)->m_step++;
            }
            break;

        case UPGRADE_STATE_SEND_SEG:
            if(_imp(_self)->m_step == CANOPEN_SEND_SEG_ADDR_STEP && (sdo_status == CO_SDO_RT_idle)){
//                printf("Send segment ADDRESS\n");
                sm_boot_canopen_send_seg_addr(_self);
                _imp(_self)->m_step++;
            }else if(_imp(_self)->m_step == CANOPEN_SEND_SEG_DATA_STEP && (sdo_status == CO_SDO_RT_idle)){
//                printf("Send segment DATA\n");
                sm_boot_canopen_send_seg_data(_self);
                _imp(_self)->m_step++;
            }else if(_imp(_self)->m_step == CANOPEN_SEND_SEG_CRC_STEP && (sdo_status == CO_SDO_RT_idle)){
//                printf("Send segment CRC\n");
                sm_boot_canopen_send_seg_crc(_self);
                _imp(_self)->m_step++;
            }
            break;

        default:
            _imp(_self)->m_state = UPGRADE_STATE_IDLE;
    }

    if(sdo_status == CO_SDO_RT_abort){
        uint8_t success = false;
//        printf("SDO status: Abort, Tx_abort: 0x%2X, Rx_Abort: 0x%2X\n", p_sdo->tx_abort_code, p_sdo->rx_abort_code);
        if(_imp(_self)->m_state == UPGRADE_STATE_SEND_SEG){
            _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback(&success,
                                                                        _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_arg);
        }else if(_imp(_self)->m_state == UPGRADE_STATE_SEND_FW_INFO){
            _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_callback(&success,
                                                                            _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_arg);
        }
        _imp(_self)->m_step = 0;
        CO_SDO_reset_status(p_sdo);
    }
    if(sdo_status == CO_SDO_RT_success){
        if(_imp(_self)->m_step >= 3){
            uint8_t success = true;
            if(_imp(_self)->m_state == UPGRADE_STATE_SEND_SEG){
                /// Uncomment for Bootloader simulator on Desktop
//                _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback(&success, _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_arg);
//                _imp(_self)->m_step = 0;
            } else if(_imp(_self)->m_state == UPGRADE_STATE_SEND_FW_INFO){
                _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_callback(&success, _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_arg);
                _imp(_self)->m_step = 0;
            }
        }
        CO_SDO_reset_status(p_sdo);
    }
    return 0;
}

/***
 *
 * @param _self
 * @return
 */
int32_t sm_boot_canopen_init(sm_boot_output_if_t* _self){
//    app_co_init();
    return 0;
}

int32_t sm_boot_canopen_free(sm_boot_output_if_t* _self){
//    app_co_free();

    _imp(_self)->m_step = 0;
    _imp(_self)->m_state = UPGRADE_STATE_IDLE;
    _imp(_self)->m_src_node_id = -1;
    _imp(_self)->m_segment = false;
    _imp(_self)->m_fw_signature = NULL;
    return 0;
}

int32_t sm_boot_canopen_reg_event_callback(sm_boot_output_if_t* _self,
                                           SM_BOOT_SLAVE_EVENT _event,
                                           sm_boot_slave_event_callback_fn_t _fn,
                                           void* _arg){
    if(!_self || !_fn){
        return -1;
    }
    _imp(_self)->m_events[_event].m_callback = _fn;
    _imp(_self)->m_events[_event].m_arg = _arg;
    return 0;
}

int32_t sm_boot_canopen_request_upgrade(sm_boot_output_if_t* _self, int32_t _src_node_id){
    if(!_self){
        return -1;
    }

    if(_imp(_self)->m_src_node_id >= 0 && _imp(_self)->m_src_node_id != _src_node_id){
        //LOG_WRN(TAG, "CanOpen is upgrading progress with other Device that Node Id = %d", _imp(_self)->m_src_node_id);
        printf("CanOpen is upgrading progress with other Device that Node Id = %d\n", _imp(_self)->m_src_node_id);
        return -1;
    }

    _imp(_self)->m_src_node_id = _src_node_id;
    _imp(_self)->m_state = UPGRADE_STATE_REQUEST_UPGRADE;

    //LOG_INF(TAG, "Request upgrade firmware");
    printf("Request upgrade firmware, Source NODE ID: %d\n", _src_node_id);

    return 0;
}

int32_t sm_boot_canopen_set_fw_info(sm_boot_output_if_t* _self, const sm_fw_signature_t* _fw_info){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    if(!_self){
        return -1;
    }
    if(_imp(_self)->m_src_node_id < 0){
        //LOG_ERR(TAG, "CanOpen is NOT active or busy");
        return -1;
    }

    _imp(_self)->m_fw_signature = _fw_info;
 /*   //LOG_INF(TAG, "New fw with version: %X.%X.%X\n size: %d \n crc: %d\n",
            _fw_info->m_version[0],
            _fw_info->m_version[1],
            _fw_info->m_version[2],
            _fw_info->m_size,
            _fw_info->m_crc);*/

    _imp(_self)->m_state = UPGRADE_STATE_SEND_FW_INFO;
    _imp(_self)->m_step = CANOPEN_SEND_FW_SIZE_STEP;

    CO_SDO_reset_status(p_sdo);

    return 0;
}

int32_t sm_boot_canopen_set_seg_fw(sm_boot_output_if_t* _self, const sm_segment_t* _seg){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    if(!_self){
        return -1;
    }
    if(_imp(_self)->m_src_node_id < 0){
        //LOG_ERR(TAG, "CanOpen is NOT active or busy");
        return -1;
    }

    _imp(_self)->m_segment = _seg;
    _imp(_self)->m_state = UPGRADE_STATE_SEND_SEG;
    _imp(_self)->m_step = CANOPEN_SEND_SEG_ADDR_STEP;

    printf("Address Segment: 0x%2X\n", _seg->m_addr);

    CO_SDO_reset_status(p_sdo);

    return 0;
}


static const sm_boot_output_if_proc_t g_boot_output_proc_default = {
        .init = sm_boot_canopen_init,
        .free = sm_boot_canopen_free,
        .reg_event_callback = sm_boot_canopen_reg_event_callback,
        .request_upgrade = sm_boot_canopen_request_upgrade,
        .set_fw_info = sm_boot_canopen_set_fw_info,
        .set_seg_fw = sm_boot_canopen_set_seg_fw,
        .process = sm_boot_canopen_process
};

static sm_boot_canopen_impl_t g_boot_canopen_default = {
        .m_base.m_proc = &g_boot_output_proc_default,
        .m_step = 0,
        .m_src_node_id = -1,
        .m_state = UPGRADE_STATE_IDLE,
        .m_segment = NULL,
        .m_fw_signature = NULL,
        .m_upgrade_request = true,
        .m_events = {NULL, NULL, NULL, NULL, NULL}
};

static uint8_t g_response_state = 0;
static uint8_t g_response_ext_boot = 0;

static CO_Sub_Object_Ext_Confirm_Func_t sm_boot_canopen_response_state_handle(void){
    if(g_response_state == BOOT_ST_LOADING_SERVER &&
        g_boot_canopen_default.m_state == UPGRADE_STATE_SEND_SEG){
//        printf("Sync state slave: %d\n", g_response_state);
        if(g_boot_canopen_default.m_step >= 3) {
            uint8_t success = true;
            g_boot_canopen_default.m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback(&success,
                                                                                  g_boot_canopen_default.m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_arg);

            g_boot_canopen_default.m_step = 0;
        }
        return CO_EXT_CONFIRM_success;
    }

    if(g_boot_canopen_default.m_state == UPGRADE_STATE_REQUEST_UPGRADE){
        uint8_t ready = true;
        g_boot_canopen_default.m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback(&ready,
                                                                                g_boot_canopen_default.m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_arg);

        printf("Confirm slave ready: %d\n", g_response_state);
    }
    return CO_EXT_CONFIRM_success;
}

static CO_Sub_Object_Ext_Confirm_Func_t sm_boot_canopen_response_ext_boot_handle(void){
    printf("Confirm external request slave ready: %d\n", g_response_ext_boot);
    if(g_boot_canopen_default.m_state == UPGRADE_STATE_REQUEST_UPGRADE){
        uint8_t ready = true;
        g_boot_canopen_default.m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback(&ready,
                                                                                g_boot_canopen_default.m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_arg);

    }
    return CO_EXT_CONFIRM_success;
}

sm_boot_output_if_t* sm_get_co_boot_output(){
    struct CO_Object_t* fw_obj = NULL;

    for(int index = 0; index < p_co_od->number; index++){
        if(p_co_od->list[index].index == SDO_BOOTLOADER_INDEX){
            fw_obj = &p_co_od->list[index];
            break;
        }
    }
    fw_obj->subs[SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX].p_data = &g_response_ext_boot;
    fw_obj->subs[SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX].p_ext->p_shadow_data = &g_response_ext_boot;
    fw_obj->subs[SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX].p_ext->confirm_func = sm_boot_canopen_response_ext_boot_handle;

    fw_obj->subs[SDO_BOOTLOADER_BOOT_STATE_SUB_INDEX].p_data = &g_response_state;
    fw_obj->subs[SDO_BOOTLOADER_BOOT_STATE_SUB_INDEX].p_ext->p_shadow_data = &g_response_state;
    fw_obj->subs[SDO_BOOTLOADER_BOOT_STATE_SUB_INDEX].p_ext->confirm_func = sm_boot_canopen_response_state_handle;

    CO_set_node_id(&CO_DEVICE, BOOT_MASTER_NODE_ID);

    g_boot_canopen_default.m_step = 0;
    g_boot_canopen_default.m_state = UPGRADE_STATE_IDLE;
    g_boot_canopen_default.m_src_node_id = -1;
    g_boot_canopen_default.m_segment = false;
    g_boot_canopen_default.m_fw_signature = NULL;

    elapsed_timer_resetz(&g_boot_canopen_default.m_timeout, PERIOD_REQUEST_UPGRADE_TIME);

    return &g_boot_canopen_default.m_base;
}
