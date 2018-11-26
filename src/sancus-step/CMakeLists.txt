set(SRC
    sancus_step.c
)

include_directories(${SANCUS_INCLUDES})
add_library(sancus-step STATIC ${SRC})
install(TARGETS sancus-step
        EXPORT ExportTargets
        ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
