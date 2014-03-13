set(SRC
    event_loop.c
)

add_library(event-loop STATIC ${SRC})
target_link_libraries(event-loop sm-control tools)
