cmake_minimum_required(VERSION 3.22)
project(bss_sdk)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

include_directories(common/
        common/interfaces/
        common/modbus/
        common/canopen/interface
        common/canopen/init
        common/canopen/od
        libs/
        libs/canopen/
        libs/host/
        libs/nanoModbus/
        services/
        utils/
        utils/timing
        utils/c-utils/linux/
        bpManagement/
        bpManagement/canopen/
        cabinetManagement/
        powerManagement/
        powerManagement/measurement/
        ota/
        ota/boot/
        ota/fw/
        api/
        app/
        app/unit-test/bp)

set(SDK_UTILS_SRC
        utils/c-utils/CRC.c
        utils/c-utils/CRC.h
        utils/c-utils/intel_hex.c
        utils/c-utils/intel_hex.h
        utils/c-utils/string_t.h
        utils/c-utils/string_t.c
        utils/c-utils/linux/linux_serial.c
        utils/c-utils/linux/tcp_client.c
        utils/c-utils/linux/tcp_server.c
        utils/FileUtils.cpp
        utils/Logger.h
        utils/Logger.cpp
        utils/timing/Timer.h
        utils/timing/elapsed_timer.h
        utils/timing/TimeUtils.h
        utils/timing/DateTime.cpp
        utils/timing/DateTime.h
        utils/utils.h
        utils/SerialPort.cpp
        utils/SerialPort.h)

add_subdirectory(utils)
add_subdirectory(common/)
add_subdirectory(common/canopen)
add_subdirectory(common/modbus)
add_subdirectory(api)
add_subdirectory(bpManagement)
add_subdirectory(cabinetManagement)
add_subdirectory(powerManagement)
add_subdirectory(libs/canopen)
add_subdirectory(libs/host)
add_subdirectory(ota)

add_subdirectory(app/unit-test/bp)
add_subdirectory(app/sampleApp)
add_subdirectory(app/function-test)
