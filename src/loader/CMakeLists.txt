set(SRC
    elf.c
    pmem.c
    global_symtab.c
)

add_library(loader STATIC ${SRC})
