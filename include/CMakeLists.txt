set(HEADERS
    elf.h
    event_loop.h
    global_symtab.h
    pmem.h
    ps2.h
    sm_control.h
    timer.h
    tools.h
    uart.h
)

install(FILES ${HEADERS} DESTINATION "${INSTALL_INCLUDES_DIR}/sancus_support")
