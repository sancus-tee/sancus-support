cmake_minimum_required(VERSION 2.8)

project(sancus-support)

set(CMAKE_C_COMPILER msp430-gcc)
include_directories ("${PROJECT_SOURCE_DIR}/include")
add_definitions("-D__MSP430F149__")

set(INSTALL_BASE_DIR     "share/sancus-support")
set(INSTALL_LIBS_DIR     "${INSTALL_BASE_DIR}/lib")
set(INSTALL_INCLUDES_DIR "${INSTALL_BASE_DIR}/include")

add_subdirectory(src)
add_subdirectory(include)
