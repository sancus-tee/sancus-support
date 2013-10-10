set(SRC
    main.c
    event_loop.c
    sm_control.c
    tools.c
)

set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld")
add_executable(main ${SRC})
target_link_libraries(main dev loader "-lsancus-sm-support" "-lsancus-host-support")
