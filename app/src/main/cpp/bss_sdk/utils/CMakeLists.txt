cmake_minimum_required(VERSION 3.22)
project(sdk-utils)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

set(SDK_UTILS_SRC
        c-utils/CRC.c
        c-utils/CRC.h
        c-utils/intel_hex.c
        c-utils/intel_hex.h
        c-utils/string_t.h
        c-utils/string_t.c
        c-utils/linux/linux_serial.c
        c-utils/linux/tcp_client.c
        c-utils/linux/tcp_server.c
        FileUtils.cpp
        Logger.h
        Logger.cpp
        timing/Timer.h
        timing/elapsed_timer.h
        timing/TimeUtils.h
        timing/DateTime.cpp
        timing/DateTime.h
        utils.h
        SerialPort.cpp
        SerialPort.h)

add_library(sdk_utils STATIC
        ${SDK_UTILS_SRC})
