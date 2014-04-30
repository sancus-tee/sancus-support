set(SRC
    link.c
    cobs.c
)

include_directories(${SANCUS_INCLUDES})
add_library(link STATIC ${SRC})
target_link_libraries(link dev)
install(TARGETS link
        EXPORT ExportTargets
        ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
