cmake_minimum_required(VERSION 3.22)
project(sampleApp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

add_executable(sampleApp
        sampleApp.cpp)

target_link_libraries(sampleApp bss_api)