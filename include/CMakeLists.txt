set(HEADERS
    sancus_support/elf.h
    sancus_support/event_loop.h
    sancus_support/global_symtab.h
    sancus_support/pmem.h
    sancus_support/ps2.h
    sancus_support/ps2_hardware.h
    sancus_support/sm_control.h
    sancus_support/timer.h
    sancus_support/tools.h
    sancus_support/uart.h
)

install(FILES ${HEADERS} DESTINATION "${INSTALL_INCLUDES_DIR}/sancus_support")
