//
// Created by vnbk on 12/09/2023.
//

#ifndef BSS_SDK_SM_OTA_BOOT_IMPL_H
#define BSS_SDK_SM_OTA_BOOT_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_boot_slave.h"

sm_boot_output_if_t* sm_get_co_boot_output();

sm_boot_input_if_t* sm_get_file_boot_input(const char* file_path);

#ifdef __cplusplus
};
#endif

#endif //BSS_SDK_SM_OTA_BOOT_IMPL_H
