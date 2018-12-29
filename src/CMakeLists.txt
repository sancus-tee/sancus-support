find_package(Sancus-Compiler REQUIRED)
find_package(MSPGCC REQUIRED)

include(UseMSPGCC)

include_directories ("${PROJECT_SOURCE_DIR}/include/sancus_support")

add_definitions(-ffunction-sections)

if(MSP430_GCC_PREFIX STREQUAL msp430-elf)
  include_directories ("${MSPGCC_BASE_DIR}/../include")
else()
  add_definitions("-D__${MCU}__")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

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
add_subdirectory(sm-io)
add_subdirectory(malloc)
add_subdirectory(tools)
add_subdirectory(main)
add_subdirectory(sancus-step)
