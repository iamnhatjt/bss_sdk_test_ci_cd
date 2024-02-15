cmake_minimum_required(VERSION 3.22)
project(bp-testing)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

add_executable(bp_upgrading
        bp_upgrading.cpp)
target_link_libraries(bp_upgrading bss_api)

add_executable(bp_assigning
        bp_assigning.cpp)
target_link_libraries(bp_assigning bpManagement pthread)

add_executable(bp_sn_writing
        bp_sn_writing.cpp)
target_link_libraries(bp_sn_writing bpManagement pthread)

