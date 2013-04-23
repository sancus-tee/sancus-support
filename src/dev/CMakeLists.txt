set(SRC
    timer.c
    uart.c
    ps2.c
)

add_library(dev STATIC ${SRC})
