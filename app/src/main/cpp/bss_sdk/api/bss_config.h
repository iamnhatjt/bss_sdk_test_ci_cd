//
// Created by vnbk on 07/08/2023.
//

#ifndef BSS_SDK_BSS_CONFIG_H
#define BSS_SDK_BSS_CONFIG_H

#define BSS_VERSION_3_0_0
//#define BSS_VERSION_3_1_0

#define BSS_CAB_NUMBER_DEFAULT      19

#ifdef BSS_VERSION_3_0_0
#define BP_CO_INTERFACE
#define CO_SERIAL_PORT_INTERFACE
//#define CO_ETHERNET_INTERFACE
#endif ///BSS_VERSION_3_0_0

#ifdef BSS_VERSION_3_1_0
#define CAB_MODBUS_INTERFACE
#define BP_MODBUS_INTERFACE
#endif ///BSS_VERSION_3_1_0


/**
 * @brief Configure for Cabinet Management Interface
 */
#define CAB_SERIAL_PORT_BAUD            115200

#define CAB_TEMP_LOWER_THRESHOLD        (10)
#define CAB_TEMP_UPPER_THRESHOLD        (70)
#define CAB_TEMP_OFFSET                 (-5)

#define CAB_POGO_TEMP_LOWER_THRESHOLD   (CAB_TEMP_LOWER_THRESHOLD)
#define CAB_POGO_TEMP_UPPER_THRESHOLD   (CAB_TEMP_UPPER_THRESHOLD)
#define CAB_POGO_TEMP_OFFSET            (0)

#define CAB_TEMP_INVALID                (-1)

/**
 * Configure for Battery Management Interface
 */
#ifdef BP_CO_INTERFACE

#ifdef CO_SERIAL_PORT_INTERFACE
#ifdef BUILD_ON_DEVICE
#define CANBUS_SERIAL_PORT        "/dev/ttyS0"
#else
#define CANBUS_SERIAL_PORT        "/dev/ttyS0"
#endif

#define CO_SERIAL_PORT_BAUD   115200
#endif ///CO_SERIAL_PORT_INTERFACE

#ifdef CO_ETHERNET_INTERFACE
#define CO_TCP_SERVER_IP    "192.168.1.254"
#define CO_TCP_SERVER_PORT  32000
#endif
#endif ///BP_CO_INTERFACE

#ifdef BP_MODBUS_INTERFACE
#define BP_MODBUS_SERIAL_PORT       "/dev/ttyUSB1"
#define BP_MODBUS_SERIAL_PORT_BAUD  115200
#endif ///BP_MODBUS_INTERFACE

/**
 * @brief Charger Configuration
 */
#define BSS_CHARGER_NUMBER_DEFAULT              2
#define LIST_CABS_MANAGED_BY_CHARGER_0_DEFAULT  {0,4,8,12,1,5,9,13,16}
#define LIST_CABS_MANAGED_BY_CHARGER_1_DEFAULT  {2,6,10,14,3,7,11,15,17,18}

/**
 * @brief Electric Meter configuration
 */
#define BSS_EMETER_UPDATE_TIME_PERIOD       (5*60*1000) ///5min

/**
 * @brief USB Serial Vendor support
 */
#define LIST_USB_SERIAL_VENDOR_SUPPORT  {"10c4", "1a86"}
#define MAX_USB_SERIAL_SUPPORT          20

int32_t findUsbSerialConverterSupport();
const char* getUsbSerialConverterSupport(int32_t _index);
int32_t getUsbSerialConverterSize();

#define BSS_LOG_PUTS_DEFAULT    true

#endif //BSS_SDK_BSS_CONFIG_H
