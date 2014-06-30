set(SRC
    packet.c
)

include_directories(${SANCUS_INCLUDES})
add_library(packet STATIC ${SRC})
target_link_libraries(packet link)
install(TARGETS packet
        EXPORT ExportTargets
        ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
