//
// Created by vnbk on 25/09/2023.
//

#ifndef BSS_SDK_BSSMODBUSDEFINE_H
#define BSS_SDK_BSSMODBUSDEFINE_H

/// BSS Modbus RTU INPUT Register: 0x04
#define BSS_MODBUS_CAB_STATUS_TEMP              0x40001
#define BSS_MODBUS_CAB_FAN_STT_INDEX                 0
#define BSS_MODBUS_CAB_CHARGER_SWITCH_STT_INDEX      1
#define BSS_MODBUS_CAB_DOOR_STT_INDEX                2

#define BSS_MODBUS_CAB_POGO_TEMP_12             0x40002
#define BSS_MODBUS_CAB_POGO_TEMP_34             0x40003

#define BSS_MODBUS_BP_CONNECTED_STATE           0x40004
#define BSS_MODBUS_BP_STATUS                    0x40005
#define BSS_MODBUS_BP_STATE                     0x40006
#define BSS_MODBUS_BP_CYCLE                     0x40007
#define BSS_MODBUS_BP_SOC_SOH                   0x40008
#define BSS_MODBUS_BP_VOL                       0x40009
#define BSS_MODBUS_BP_CUR                       0x4000A

#define BSS_MODBUS_BP_CELL_0                    0x4000B
#define BSS_MODBUS_BP_CELL_1                    0x4000C
#define BSS_MODBUS_BP_CELL_2                    0x4000D
#define BSS_MODBUS_BP_CELL_3                    0x4000E
#define BSS_MODBUS_BP_CELL_4                    0x4000F
#define BSS_MODBUS_BP_CELL_5                    0x40010
#define BSS_MODBUS_BP_CELL_6                    0x40011
#define BSS_MODBUS_BP_CELL_7                    0x40012
#define BSS_MODBUS_BP_CELL_8                    0x40013
#define BSS_MODBUS_BP_CELL_9                    0x40014
#define BSS_MODBUS_BP_CELL_10                   0x40015
#define BSS_MODBUS_BP_CELL_11                   0x40016
#define BSS_MODBUS_BP_CELL_12                   0x40017
#define BSS_MODBUS_BP_CELL_13                   0x40018
#define BSS_MODBUS_BP_CELL_14                   0x40019
#define BSS_MODBUS_BP_CELL_15                   0x4001A

#define BSS_MODBUS_BP_ZONE_TEMP_01              0x4001B
#define BSS_MODBUS_BP_ZONE_TEMP_23              0x4001C
#define BSS_MODBUS_BP_ZONE_TEMP_45              0x4001D
#define BSS_MODBUS_BP_ZONE_TEMP_6               0x4001E

#define BSS_MODBUS_BP_SN                        0x4001F
#define BSS_MODBUS_BP_VERSION                   0x4002E

#define BSS_MODBUS_BP_UPGRADE_STATUS            0x4003D
#define BSS_MODBUS_BP_CANBUS_RX_FIFO_0          0x4003E
#define BSS_MODBUS_BP_CANBUS_RX_FIFO_1          0x40043
#define BSS_MODBUS_BP_CANBUS_RX_FIFO_2          0x40048
#define BP_CANBUS_RX_REG_SIZE                   5

enum {
    BSS_MODBUS_CAB_STATUS_TEMP_REG_INDEX = 0,
    BSS_MODBUS_CAB_POGO_TEMP_12_REG_INDEX,
    BSS_MODBUS_CAB_POGO_TEMP_34_REG_INDEX,

    BSS_MODBUS_BP_CONNECTED_STATE_REG_INDEX,
    BSS_MODBUS_BP_STATUS_REG_INDEX,
    BSS_MODBUS_BP_STATE_REG_INDEX,
    BSS_MODBUS_BP_CYCLE_REG_INDEX,
    BSS_MODBUS_BP_SOC_SOH_REG_INDEX,
    BSS_MODBUS_BP_VOL_REG_INDEX,
    BSS_MODBUS_BP_CUR_REG_INDEX,
    BSS_MODBUS_BP_CELL_0_REG_INDEX,
    BSS_MODBUS_BP_CELL_1_REG_INDEX,
    BSS_MODBUS_BP_CELL_2_REG_INDEX,
    BSS_MODBUS_BP_CELL_3_REG_INDEX,
    BSS_MODBUS_BP_CELL_4_REG_INDEX,
    BSS_MODBUS_BP_CELL_5_REG_INDEX,
    BSS_MODBUS_BP_CELL_6_REG_INDEX,
    BSS_MODBUS_BP_CELL_7_REG_INDEX,
    BSS_MODBUS_BP_CELL_8_REG_INDEX,
    BSS_MODBUS_BP_CELL_9_REG_INDEX,
    BSS_MODBUS_BP_CELL_10_REG_INDEX,
    BSS_MODBUS_BP_CELL_11_REG_INDEX,
    BSS_MODBUS_BP_CELL_12_REG_INDEX,
    BSS_MODBUS_BP_CELL_13_REG_INDEX,
    BSS_MODBUS_BP_CELL_14_REG_INDEX,
    BSS_MODBUS_BP_CELL_15_REG_INDEX,

    BSS_MODBUS_BP_ZONE_TEMP_01_REG_INDEX,
    BSS_MODBUS_BP_ZONE_TEMP_23_REG_INDEX,
    BSS_MODBUS_BP_ZONE_TEMP_45_REG_INDEX,
    BSS_MODBUS_BP_ZONE_TEMP_6_REG_INDEX,

    BSS_MODBUS_BP_SN_REG_INDEX,
    BSS_MODBUS_BP_VERSION_REG_INDEX,

    BSS_MODBUS_INPUT_REG_NUMBER
};

#define BSS_MODBUS_CAB_INPUT_REG_NUMBER     3
#define BSS_MODBUS_BP_INPUT_REG_NUMBER      (BSS_MODBUS_INPUT_REG_NUMBER - BSS_MODBUS_CAB_INPUT_REG_NUMBER)
#define BSS_MODBUS_SYNC_REG_NUMBER          30

/// BSS Modbus RTU Holding Register: 0x03
#define BSS_MODBUS_CAB_OPEN_DOOR                0x30003
#define BSS_MODBUS_CAB_CTL_FAN                  0x30002
#define BSS_MODBUS_CAB_CTL_CHARGER_SWITCH       0x30001
#define BSS_MODBUS_CAB_CTL_LED                  0x30004
#define BSS_MODBUS_CAB_REBOOT                   0x30005

#define BSS_MODBUS_BP_ASSIGNING                 0x30006
#define BSS_MODBUS_BP_REBOOT                    0x30007
#define BSS_MODBUS_BP_STATE_CONTROL             0x30008
#define BSS_MODBUS_BP_CHARGER_SWITCH            0x30009
#define BSS_MODBUS_BP_ASSIGNED_DEVICE           0x3000B

#define BSS_MODBUS_BP_CANBUS_TX_FIFO            0x3001B
#define BP_CANBUS_TX_REG_SIZE                   BP_CANBUS_RX_REG_SIZE

enum {
    BSS_MODBUS_CAB_OPEN_DOOR_REG_INDEX = 0,
    BSS_MODBUS_CAB_CTL_FAN_REG_INDEX,
    BSS_MODBUS_CAB_CTL_CHARGER_SWITCH_REG_INDEX,
    BSS_MODBUS_CAB_CTL_LED_REG_INDEX,
    BSS_MODBUS_CAB_REBOOT_REG_INDEX,

    BSS_MODBUS_BP_ASSIGNING_REG_INDEX,
    BSS_MODBUS_BP_REBOOT_REG_INDEX,
    BSS_MODBUS_BP_STATE_CONTROL_REG_INDEX,
    BSS_MODBUS_BP_CHARGER_SWITCH_REG_INDEX,
    BSS_MODBUS_BP_ASSIGNED_DEVICE_REG_INDEX,

    BSS_MODBUS_HOLDING_REG_NUMBER
};

#define BSS_MODBUS_TURN_ON                      (0x01)
#define BSS_MODBUS_TURN_OFF                     (0x00)
#define BSS_MODBUS_OPEN_DOOR                    BSS_MODBUS_TURN_ON

#endif //BSS_SDK_BSSMODBUSDEFINE_H
