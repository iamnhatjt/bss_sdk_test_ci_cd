cmake_minimum_required(VERSION 3.22)
project(power_management)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

add_library(chargerManagement STATIC
        ChargerManager.cpp
        ChargerManager.h
        ChargerCom.cpp
        ChargerCom.h)

target_link_libraries(chargerManagement sdk_utils)

add_library(powerMeasurement STATIC
        measurement/ModbusEMeterManager.cpp
        measurement/ModbusEMeterManager.h
        measurement/Vse11ModbusEMeter.h
        measurement/Dds6619ModbusEMeter.h)

target_link_libraries(powerMeasurement modbus)