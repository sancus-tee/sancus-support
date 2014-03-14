file(RELATIVE_PATH REL_INCLUDES_DIR ${INSTALL_CMAKE_DIR} ${INSTALL_INCLUDES_DIR})
# file(RELATIVE_PATH REL_LIBS_DIR ${INSTALL_CMAKE_DIR} ${INSTALL_LIBS_DIR})
configure_file(Sancus-SupportConfig.cmake.in Sancus-SupportConfig.cmake @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Sancus-SupportConfig.cmake"
        DESTINATION ${INSTALL_CMAKE_DIR})
