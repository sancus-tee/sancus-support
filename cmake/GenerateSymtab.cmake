include(CMakeParseArguments)

function(generate_symtab)
    if (NOT EXISTS ${SANCUS_SUPPORT_GENSYMTAB})
        message(FATAL_ERROR "Cannot find suitable symtab generator script")
    endif ()

    cmake_parse_arguments(ARG "" "" "STD_LIBS" ${ARGN})
    list(GET ARG_UNPARSED_ARGUMENTS 0 output)
    list(GET ARG_UNPARSED_ARGUMENTS 1 input)

    # the list of input files is the given target library and all its
    # dependencies
    set(inputs $<TARGET_FILE:${input}>)
    get_target_property(libs ${input} INTERFACE_LINK_LIBRARIES)

    foreach (lib ${libs})
        if (NOT TARGET ${lib})
            message(FATAL_ERROR "Only targets are supported as libraries")
        endif ()

        set(inputs ${inputs} $<TARGET_FILE:${lib}>)
    endforeach ()

    set(options -o ${output} ${inputs})

    foreach (lib ${ARG_STD_LIBS})
        set(options ${options} --add-std-lib ${lib})
    endforeach ()

    add_custom_command(OUTPUT  ${output}
                       COMMAND ${SANCUS_SUPPORT_GENSYMTAB} ${options}
                       DEPENDS ${input} ${libs})
endfunction()
