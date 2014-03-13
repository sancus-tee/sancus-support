set(SRC
    elf.c
    pmem.c
    global_symtab.c
)

include_directories(${SANCUS_INCLUDES})
add_library(loader STATIC ${SRC})
