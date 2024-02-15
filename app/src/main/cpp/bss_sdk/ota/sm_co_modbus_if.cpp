//
// Created by vnbk on 09/10/2023.
//
#include <stdio.h>

#include "sm_co_if.h"
#include "Logger.h"

#include "ModbusMasterInterface.h"
#include "BssModbusDefine.h"

#define PACKET_LENGTH   20

#define TAG "sm_co_modbus_if"

struct sm_co_if{
    int32_t m_baud;
    int32_t m_current_address;
    void* m_modbus_master;
    sm_co_if_receive_callback_fn m_callback;
};

static sm_co_if_t g_canopen_if; /*= {
        .m_baud = 115200,
        .m_callback = NULL,
};*/

sm_co_if_t *sm_co_if_create(uint8_t _type, const char *host, int port){
    return NULL;
}

sm_co_if_t *sm_co_if_create_default(uint8_t _type, const char *host, int port){
    if(_type == MODBUS_CANBUS) {
        g_canopen_if.m_baud = port;

        return &g_canopen_if;
    }
    return NULL;
}

int sm_co_if_free(sm_co_if_t* self){
    if(!self){
        return -1;
    }

    return 0;
}

int sm_co_if_reg_recv_callback(sm_co_if_t* self, sm_co_if_receive_callback_fn callback_fn){
    if(!self){
        return -1;
    }
    self->m_callback = callback_fn;
    return 0;
}

int sm_co_if_connect(sm_co_if_t *self){
    return 0;
}

int sm_co_if_disconnect(sm_co_if_t *self){
    return 0;
}

int sm_co_if_is_connected(sm_co_if_t *self){
    return 0;
}

int sm_co_if_send(sm_co_if_t *self, uint32_t frame_id, const unsigned char *data, int len, int timeout){
    if(!self || !(self->m_modbus_master)){
        return -1;
    }
    uint16_t buf[PACKET_LENGTH] = {0,};
    int index = 0;
    len = (len%2) ? (len+1) : len;

    buf[index++] = (uint16_t)(frame_id & 0xFFFF);
    for(int i = 0; i < len; i++){
        buf[index] = ((uint16_t)data[i] << 8) | (uint16_t)(data[i+1]);
        index++;
    }

   /* if(frame_id>=0x500){
        printf("Send CanOpen FrameId: 0x%2X\n", frame_id);
    }*/

    ModbusMasterInterface* mbMaster = (ModbusMasterInterface*)self->m_modbus_master;
    int32_t ret = mbMaster->writeMultipleRegisters(self->m_current_address,
                                                   MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_CANBUS_RX_FIFO_0),
                                                   BP_CANBUS_TX_REG_SIZE,
                                                   buf);

    if(ret != MODBUS_ERROR_NONE){
        LOG_ERROR(TAG, "Write multiples CANBUS register via Modbus FAILURE");
        return -1;
    }
    return len;
}

int sm_co_if_recv(sm_co_if_t *self, unsigned char *buf, int max_len, int timeout){
    return 0;
}

int sm_co_if_process(sm_co_if_t *self){
    if(!self || !self->m_callback) {
        return -1;
    }

    ModbusMasterInterface* mbMaster = (ModbusMasterInterface*)self->m_modbus_master;
    uint16_t value[PACKET_LENGTH];

    int32_t ret = mbMaster->readInputRegisters(self->m_current_address,
                                               MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_UPGRADE_STATUS),
                                               1,
                                               value);

    if(ret != MODBUS_ERROR_NONE){
        return -1;
    }

    uint8_t fifoAvailable = value[0] & 0xFF;
    if(!fifoAvailable){
        return 0;
    }

    LOG_DEBUG(TAG, "Fifo available: %d canbus message", fifoAvailable);

    ret = mbMaster->readInputRegisters(self->m_current_address,
                                       MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_CANBUS_RX_FIFO_0),
                                       fifoAvailable,
                                       value);

    if(ret != MODBUS_ERROR_NONE){
        return -1;
    }

    while (fifoAvailable--){
        self->m_callback(value[0], (uint8_t*)&value[1*(fifoAvailable-1)]);
    }

    return 0;
}