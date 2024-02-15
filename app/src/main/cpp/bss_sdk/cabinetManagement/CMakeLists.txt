cmake_minimum_required(VERSION 3.22)
project(cabinet_management)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

add_library(cabManagement STATIC
        ${SDK_UTILS_SRC_SOURCE_DIR}
        CabManagement.h
        CabManagement.cpp
        CabCommunication.cpp
        CabCommunication.h
        ../common/BssModbusCom.cpp)

target_link_libraries(cabManagement sdk_utils)