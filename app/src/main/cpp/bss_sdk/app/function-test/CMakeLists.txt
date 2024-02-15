cmake_minimum_required(VERSION 3.22)
project(charger-testing)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

add_executable(chargerTesting
        chargerTesting.cpp)

target_link_libraries(chargerTesting bss_api)