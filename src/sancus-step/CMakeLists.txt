set(CMAKE_C_COMPILER sancus-cc)
set(CMAKE_C_CREATE_STATIC_LIBRARY "sancus-ld --verbose <OBJECTS> -o <TARGET>")

set(SRC
    sancus_step.c
)

include_directories(${SANCUS_INCLUDES})
add_library(sancus-step STATIC ${SRC})
install(TARGETS sancus-step
        EXPORT ExportTargets
        ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
