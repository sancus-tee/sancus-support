function(generate_symtab output input)
    if (NOT EXISTS ${SANCUS_SUPPORT_GENSYMTAB})
        message(FATAL_ERROR "Cannot find suitable symtab generator script")
    endif ()

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

    add_custom_command(OUTPUT  ${output}
                       COMMAND ${SANCUS_SUPPORT_GENSYMTAB} -o ${output} ${inputs}
                       DEPENDS ${input} ${libs})
endfunction()
