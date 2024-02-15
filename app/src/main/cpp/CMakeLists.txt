# For more information about using CMake with Android Studio, read the
# documentation: https:/d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22)

# Declares and names the project.

project("bss_sdk_wrapper")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

add_compile_options(-fPIC)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

include_directories(bss_sdk/
        bss_sdk/api/
        bss_sdk/cabinetManagement/
        bss_sdk/powerManagement/
        bss_sdk/powerManagement/measurement/
        bss_sdk/ota/
        bss_sdk/ota/boot/
        bss_sdk/ota/fw/
        bss_sdk/bpManagement/
        bss_sdk/bpManagement/canopen/
        bss_sdk/common/
        bss_sdk/common/interfaces/
        bss_sdk/common/modbus/
        bss_sdk/common/canopen/
        bss_sdk/common/canopen/init/
        bss_sdk/common/canopen/interface/
        bss_sdk/common/canopen/od/
        bss_sdk/common/canopen/od/comm_od/
        bss_sdk/common/canopen/od/manu_od/
        bss_sdk/libs/
        bss_sdk/libs/canopen/
        bss_sdk/libs/host/
        bss_sdk/libs/nanoModbus/
        bss_sdk/utils/
        bss_sdk/utils/c-utils/
        bss_sdk/utils/c-utils/linux/
        bss_sdk/utils/threading/
        bss_sdk/utils/timing)

set(SDK_COMMON_SRC
        bss_sdk/utils/c-utils/CRC.c
        bss_sdk/utils/c-utils/CRC.h
        bss_sdk/utils/c-utils/string_t.c
        bss_sdk/utils/c-utils/string_t.h
        bss_sdk/utils/c-utils/intel_hex.c
        bss_sdk/utils/c-utils/intel_hex.h
        bss_sdk/utils/FileUtils.cpp
        bss_sdk/utils/Logger.h
        bss_sdk/utils/Logger.cpp
        bss_sdk/utils/timing/Timer.h
        bss_sdk/utils/timing/elapsed_timer.h
        bss_sdk/utils/timing/TimeUtils.h
        bss_sdk/utils/timing/DateTime.cpp
        bss_sdk/utils/timing/DateTime.h
        bss_sdk/utils/c-utils/linux/linux_serial.h
        bss_sdk/utils/c-utils/linux/linux_serial.c
        bss_sdk/utils/c-utils/linux/tcp_client.c)
#        utils/linux/tcp_server.c)

add_subdirectory(bss_sdk/utils)
add_subdirectory(bss_sdk/common/)
add_subdirectory(bss_sdk/common/canopen)
add_subdirectory(bss_sdk/common/modbus)
add_subdirectory(bss_sdk/bpManagement)
add_subdirectory(bss_sdk/cabinetManagement)
add_subdirectory(bss_sdk/api)
add_subdirectory(bss_sdk/libs/canopen)
add_subdirectory(bss_sdk/libs/host)
add_subdirectory(bss_sdk/powerManagement)
add_subdirectory(bss_sdk/ota)

add_library( # Sets the name of the library.
        bss_sdk_wrapper

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp
        ${SDK_COMMON_SRC})

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        bss_sdk_wrapper

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})

target_link_libraries(bss_sdk_wrapper bss_api)
