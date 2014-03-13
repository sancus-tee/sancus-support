set(SRC
    tools.c
)

add_library(tools STATIC ${SRC})
install(TARGETS tools ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
