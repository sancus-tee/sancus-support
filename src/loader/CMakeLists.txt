set(SRC
    elf.c
    pmem.c
    global_symtab.c
)

include_directories(${SANCUS_INCLUDES})
add_library(loader STATIC ${SRC})
install(TARGETS loader
        EXPORT ExportTargets
        ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
