cmake_minimum_required(VERSION 3.22)
project(modbus)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

set(MODBUS_LIB_SRC
        ../../libs/nanoModbus/nanomodbus.c
        ../../libs/nanoModbus/nanomodbus.h
       )

add_library(modbus STATIC
        ${MODBUS_LIB_SRC}
        ModbusComImpl.cpp
        ModbusComImpl.h
        ModbusDefine.h
        ModbusMasterImpl.cpp
        ModbusMasterImpl.h
        )
