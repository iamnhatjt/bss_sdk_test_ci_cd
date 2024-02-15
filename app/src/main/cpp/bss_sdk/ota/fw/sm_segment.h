//
// Created by vnbk on 22/03/2023.
//

#ifndef SM_SEGMENT_H
#define SM_SEGMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define SM_SEGMENT_FW_SIZE      (1024)

typedef struct sm_segment{
    int32_t m_addr;
    int32_t m_index;
    int32_t m_size;
    int32_t m_length;
    uint8_t m_data[SM_SEGMENT_FW_SIZE];
    uint16_t m_crc;
    uint8_t m_is_last;
}sm_segment_t;

sm_segment_t *sm_seg_create();
sm_segment_t *sm_seg_create_default();
int32_t sm_seg_destroy(sm_segment_t* self);

int32_t sm_seg_set_info(sm_segment_t* self, int32_t index, int32_t addr, int32_t size, uint16_t crc);
int32_t sm_seg_fill_data(sm_segment_t* self, const uint8_t* data, int32_t len);
int32_t sm_seg_set_last_segment(sm_segment_t* self, uint8_t is_last);
int32_t sm_seg_set_addr(sm_segment_t* self, int32_t addr);
int32_t sm_seg_set_size(sm_segment_t* self, int32_t size);
int32_t sm_seg_set_crc(sm_segment_t* self, uint16_t crc);
int32_t sm_seg_is_valid(sm_segment_t* self);
int32_t sm_seg_is_full(sm_segment_t* self);

int32_t sm_seg_get_index(sm_segment_t* self);
int32_t sm_seg_get_addr(sm_segment_t* self);
int32_t sm_seg_get_crc(sm_segment_t* self);
int32_t sm_seg_get_length(sm_segment_t* self);
int32_t sm_seg_get_last_segment(sm_segment_t* self);
uint8_t* sm_seg_get_data(sm_segment_t* self);

#ifdef __cplusplus
};
#endif

#endif //SM_SEGMENT_H
