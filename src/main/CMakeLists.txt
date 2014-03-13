set(SRC
    main.c
    event_loop.c
    sm_control.c
    tools.c
)

include_directories(${SANCUS_INCLUDES})
set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld")
add_executable(main ${SRC})
target_link_libraries(main dev loader ${SANCUS_HOST_LIBS} ${SANCUS_MODULE_LIBS})
