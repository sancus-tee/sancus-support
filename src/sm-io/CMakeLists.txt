set(SRC
    sm_io.c
)

include_directories(${SANCUS_INCLUDES})
add_library(sm-io STATIC ${SRC} $<TARGET_OBJECTS:obj-uart>)
install(TARGETS sm-io
        EXPORT ExportTargets
        ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
