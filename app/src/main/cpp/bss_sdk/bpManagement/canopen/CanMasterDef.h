//
// Created by vnbk on 08/07/2023.
//

#ifndef BSS_SDK_CANMASTERDEF_H
#define BSS_SDK_CANMASTERDEF_H

#define SDO_READ_OBJ_TIMEOUT_MS							5000
#define SDO_WRITE_OBJ_TIMEOUT_MS						5000

#define BMS_INDEX										0x2003
#define BMS_SERIAL_NUMBER_OBJECT_SUB_INDEX				0x00
#define BMS_MAIN_SWITCH_SUB_INDEX						0x01
#define SLAVE_ID_NUMBER_OBJECT_SUB_INDEX				0x02

#define BMS_VERSION_INDEX								0x100A
#define BMS_VERSION_SUB_INDEX							0x00

#define BMS_VEHICLE_SN_INDEX							0x2004
#define BMS_MATTED_DEV_SUB_INDEX						0x01

#define SM_DEVICE_BOOT_INDEX                            0x2001
#define SN_DEVICE_REBOOT_SUB_INDEX                      0x07

#define BMS_STATE_CHARGING								2
#define BMS_STATE_DISCHARGING							0

#endif //BSS_SDK_CANMASTERDEF_H
