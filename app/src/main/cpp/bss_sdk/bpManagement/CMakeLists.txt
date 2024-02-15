cmake_minimum_required(VERSION 3.22)
project(bp_management)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

set(BP_CO_INTERFACE ON)

set(BP_IF_IMPL_SRC
        canopen/BpCoCom.cpp
        canopen/BpCoCom.h
        canopen/BpAssignModule.cpp
        canopen/BpAssignModule.h
        canopen/CanMasterDef.h
)

add_library(bpManagement STATIC
        ${BP_IF_IMPL_SRC}
        Bp.h
        BpManagement.cpp
        BpManagement.h)

target_link_libraries(bpManagement sdk_utils)

if(BP_CO_INTERFACE)
target_link_libraries(bpManagement co_impl host)
endif ()