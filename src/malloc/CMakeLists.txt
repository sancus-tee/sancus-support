option(TRACE_MALLOC "Enable debug tracing of memory allocations")
set(TRACE_MALLOC_NUM_ALLOCS 32
    CACHE STRING "The number of allocations to trace")

if (TRACE_MALLOC)
    add_definitions(-DTRACE_MALLOC
                    -DTRACE_MALLOC_NUM_ALLOCS=${TRACE_MALLOC_NUM_ALLOCS})
endif()

set(SRC
    malloc.c
)

include_directories(${SANCUS_INCLUDES})
add_library(malloc STATIC ${SRC})
install(TARGETS malloc
        EXPORT ExportTargets
        ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
