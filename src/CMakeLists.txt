find_package(Sancus-Compiler REQUIRED)

set(CMAKE_C_COMPILER msp430-gcc)
include_directories ("${PROJECT_SOURCE_DIR}/include/sancus_support")
add_definitions("-D__MSP430F149__" -ffunction-sections)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall")

option(DEBUG_PRINTS "Enable debug prints")

if (DEBUG_PRINTS)
    add_definitions("-DDEBUG_PRINTS")
endif ()

add_subdirectory(dev)
add_subdirectory(link)
add_subdirectory(packet)
add_subdirectory(loader)
add_subdirectory(event-loop)
add_subdirectory(sm-control)
add_subdirectory(malloc)
add_subdirectory(tools)
add_subdirectory(main)
