# configure config file for the install tree
file(RELATIVE_PATH REL_INCLUDES_DIR ${INSTALL_CMAKE_DIR} ${INSTALL_INCLUDES_DIR})
file(RELATIVE_PATH REL_TOOLS_DIR ${INSTALL_CMAKE_DIR} ${INSTALL_TOOLS_DIR})
set(CONF_INSTALL_TREE
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Sancus-SupportConfig.cmake")
configure_file(Sancus-SupportConfig.cmake.in ${CONF_INSTALL_TREE} @ONLY)

# configure config file for the build tree
message("${CMAKE_CURRENT_BINARY_DIR} -> ${PROJECT_SOURCE_DIR}/include")
file(RELATIVE_PATH REL_INCLUDES_DIR
     ${CMAKE_CURRENT_BINARY_DIR} "${PROJECT_SOURCE_DIR}/include")
file(RELATIVE_PATH REL_TOOLS_DIR
     ${CMAKE_CURRENT_BINARY_DIR} "${PROJECT_SOURCE_DIR}/tools")

configure_file(Sancus-SupportConfig.cmake.in Sancus-SupportConfig.cmake @ONLY)
configure_file(GenerateSymtab.cmake GenerateSymtab.cmake COPYONLY)

export(TARGETS dev dev-timer dev-uart dev-ps2 dev-pmodcls dev-pmodkypd dev-spi
               link packet event-loop loader sm-control tools malloc
       NAMESPACE sancus_support_
       FILE "${CMAKE_CURRENT_BINARY_DIR}/ExportTargets.cmake")

install(FILES ${CONF_INSTALL_TREE} GenerateSymtab.cmake
        DESTINATION ${INSTALL_CMAKE_DIR})
