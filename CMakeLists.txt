cmake_minimum_required(VERSION 2.8)

project(msp-support)

set(CMAKE_C_COMPILER msp430-gcc)
include_directories ("${PROJECT_SOURCE_DIR}/include")
add_definitions("-D__MSP430F149__")

add_subdirectory(src)
