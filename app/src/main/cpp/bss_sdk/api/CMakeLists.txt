cmake_minimum_required(VERSION 3.22)
project(bss-api)

set(CMAKE_CXX_STANDARD 11)
set (CMAKE_C_STANDARD 99)

add_library(bss_api STATIC
        bss_api.cpp
        bss_data_def.h
        BssDevice.cpp
        BssDevice.h
        bss_config.h
        bss_config.cpp)

target_link_libraries(bss_api cabManagement bpManagement chargerManagement ota powerMeasurement)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(bss_api -lpthread)
endif ()
