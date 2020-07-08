set(HEADERS
    sancus_support/dma_attacker.h
    sancus_support/elf.h
    sancus_support/event_loop.h
    sancus_support/global_symtab.h
    sancus_support/pmem.h
    sancus_support/ps2.h
    sancus_support/ps2_hardware.h
    sancus_support/sm_control.h
    sancus_support/timer.h
    sancus_support/tsc.h
    sancus_support/tools.h
    sancus_support/uart.h
    sancus_support/led_digits.h
    sancus_support/link.h
    sancus_support/packet.h
    sancus_support/sm_io.h
    sancus_support/spi.h
    sancus_support/pmodcls.h
    sancus_support/pmodkypd.h
    sancus_support/fileio.h
    sancus_support/sancus_step.h
)

set(BASE_INSTALL_DIR "${INSTALL_INCLUDES_DIR}/sancus_support")
install(FILES ${HEADERS} DESTINATION ${BASE_INSTALL_DIR})
install(FILES sancus_support/private/symbol.h
        DESTINATION ${BASE_INSTALL_DIR}/private)
