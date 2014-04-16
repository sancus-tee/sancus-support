include(CMakeParseArguments)

function(generate_symtab output)
    if (NOT EXISTS ${SANCUS_SUPPORT_GENSYMTAB})
        message(FATAL_ERROR "Cannot find suitable symtab generator script")
    endif ()

    cmake_parse_arguments(ARG "" "" "INPUTS;STD_LIBS" ${ARGN})

    foreach (input ${ARG_INPUTS})
        if (TARGET ${input})
            set(inputs ${inputs} $<TARGET_FILE:${input}>)
        elseif (EXISTS ${input})
            set(inputs ${inputs} ${input})
        else ()
            message(FATAL_ERROR "Unable to find input ${input}")
        endif ()
    endforeach ()

    set(options -o ${output} ${inputs})

    foreach (lib ${ARG_STD_LIBS})
        set(options ${options} --add-std-lib ${lib})
    endforeach ()

    add_custom_command(OUTPUT  ${output}
                       COMMAND ${SANCUS_SUPPORT_GENSYMTAB} ${options}
                       DEPENDS ${ARG_INPUTS})
endfunction()

function(add_executable_with_symtab name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LIBS" ${ARGN})
    set(sources ${ARG_SOURCES})
    set(libs ${ARG_LIBS})
    set(objects_lib ${name}_objects)
    set(objects $<TARGET_OBJECTS:${objects_lib}>)
    set(reloc_exec ${name}.o)

    add_library(${objects_lib} OBJECT ${sources})
    add_executable(${reloc_exec} ${objects})
    generate_symtab(symtab.c INPUTS ${reloc_exec} ${libs} STD_LIBS c)
    add_executable(${name} symtab.c ${objects})
    target_link_libraries(${name} ${libs} --standalone)
endfunction()
