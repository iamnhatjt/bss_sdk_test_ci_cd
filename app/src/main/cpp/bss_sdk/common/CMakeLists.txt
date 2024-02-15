cmake_minimum_required(VERSION 3.22)
project(bss_modbus)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

add_library(bss_modbus STATIC
        BssModbusCom.cpp
        BssModbusCom.h
        BssModbusDefine.h)

target_link_libraries(bss_modbus modbus)
