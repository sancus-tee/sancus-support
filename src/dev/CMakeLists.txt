set(SRC
    timer.c
    uart.c
    ps2.c
)

add_library(dev STATIC ${SRC})
install(TARGETS dev ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
