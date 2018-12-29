set(SRC
    main.c
)

include_directories(${SANCUS_INCLUDES})

if(MSP430_GCC_PREFIX STREQUAL msp430-elf)
  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "") # Removes unsupported -rdynamic flag
  set(CMAKE_EXE_LINKER_FLAGS "-L ${MSPGCC_BASE_DIR}/../include") 
  set(CMAKE_EXE_LINKER_FLAGS 
      "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_CURRENT_SOURCE_DIR}")
else()
  set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld")
endif()

add_executable(main ${SRC})

target_link_libraries(main dev event-loop)
