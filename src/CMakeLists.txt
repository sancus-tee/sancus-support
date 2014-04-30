find_package(Sancus-Compiler REQUIRED)

set(CMAKE_C_COMPILER msp430-gcc)
include_directories ("${PROJECT_SOURCE_DIR}/include/sancus_support")
add_definitions("-D__MSP430F149__")

add_subdirectory(dev)
add_subdirectory(link)
add_subdirectory(loader)
add_subdirectory(event-loop)
add_subdirectory(sm-control)
add_subdirectory(tools)
add_subdirectory(main)
