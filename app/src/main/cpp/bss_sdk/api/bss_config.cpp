//
// Created by vnbk on 14/08/2023.
//
#include <string.h>
#include <string>
#include <vector>

#include "Logger.h"

#include "bss_config.h"

#define TAG "bss_config"

#define ROOT_DEV    "/dev/"

#ifndef __linux
#define USB_SERIAL_VENDOR_FILE_DEFAULT  "/sys/bus/usb/devices/1-1.%d/idVendor"
#else
#define USB_SERIAL_VENDOR_FILE_DEFAULT      "/sys/bus/usb/devices/1-1.%d/idVendor"
#define CMD_FIND_USB_SERIAL_PORT_DEFAULT    "find  /sys/bus/usb/devices/1-1.%d/ -name ttyUSB*"
#endif

static int g_found = 0;
static char g_usb_dev[MAX_USB_SERIAL_SUPPORT][20];

int32_t findUsbSerialConverterSupport(){
    char filename[256];
    FILE *fp;

    std::vector<const char*> vid{LIST_USB_SERIAL_VENDOR_SUPPORT};

    int usb_port[10];
    char search_cmd[256];
    char path[1024];
    char *device_path;
    int rs = 0;

    for (int i = 1; i < MAX_USB_SERIAL_SUPPORT; i++) {
        sprintf(filename, USB_SERIAL_VENDOR_FILE_DEFAULT, i);
        LOG_DEBUG(TAG, "Filename of USB Port: %s", filename);

        fp = fopen(filename, "r");
        if(fp){
            fseek(fp, 0, SEEK_END);
            long fsize = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            char *buffer = (char*)malloc(fsize + 1);
            fread(buffer, fsize, 1, fp);

            LOG_DEBUG(TAG, "Vendor ID: %s", buffer);

            for(auto & j : vid){
                if(memcmp(buffer, j, 4) == 0){
                    usb_port[g_found] = i;
                    g_found++;
                    break;
                }
            }
            free(buffer);
            fclose(fp);
        } else {
            LOG_ERROR(TAG, "Open USB device file FAILURE");
        }
    }

    LOG_INFO(TAG, "Start search for serial port that is match. Number port found: %d", g_found);

    for(int k = 0; k < g_found; k++){
        sprintf(search_cmd, CMD_FIND_USB_SERIAL_PORT_DEFAULT, usb_port[k]);
        fp = popen(search_cmd, "r");
        if(!fp) {
            LOG_ERROR(TAG, "Could NOT run CMD");
            continue;
        } else {
            while (fgets(path, sizeof(path), fp) != nullptr) {
                device_path = strstr(path, "ttyUSB");
                strcpy(g_usb_dev[rs], ROOT_DEV);
                memcpy(g_usb_dev[rs] + strlen(ROOT_DEV), &path[device_path - path], 7);

                LOG_INFO(TAG, "USB device path: %s", g_usb_dev[rs]);
                rs++;
            }
            pclose(fp);
        }
    }
    return g_found;
}

const char* getUsbSerialConverterSupport(int32_t _index){
    if(_index >= g_found){
        return "";
    }
    return g_usb_dev[_index];
}

int32_t getUsbSerialConverterSize(){
    return g_found;
}