cmake_minimum_required(VERSION 3.22)
project(ota_service)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

add_library(ota STATIC
        boot/sm_boot_master.c
        boot/sm_boot_master.h
        boot/sm_boot_slave.h
        boot/sm_boot_slave.c
        fw/sm_fw.c
        fw/sm_segment.c
        fw/sm_fw.h
        fw/sm_segment.h
        sm_canopen_boot_output.c
        sm_file_boot_input.c
        OtaService.cpp
        OtaService.h
        sm_ota_boot_impl.h
        OtaFwManager.cpp
        OtaFwManager.h)

target_link_libraries(ota sdk_utils)