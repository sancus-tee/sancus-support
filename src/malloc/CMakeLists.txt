set(SRC
    malloc.c
)

include_directories(${SANCUS_INCLUDES})
add_library(malloc STATIC ${SRC})
install(TARGETS malloc
        EXPORT ExportTargets
        ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
