cmake_minimum_required(VERSION 2.8.8)

project(Sancus-Support)

set(INSTALL_BASE_DIR     "share/sancus-support")
set(INSTALL_LIBS_DIR     "${INSTALL_BASE_DIR}/lib")
set(INSTALL_INCLUDES_DIR "${INSTALL_BASE_DIR}/include")
set(INSTALL_CMAKE_DIR    "${INSTALL_BASE_DIR}/cmake")
set(INSTALL_TOOLS_DIR    "${INSTALL_BASE_DIR}/tools")

# convert all the above variables to absolute paths
foreach (type BASE LIBS INCLUDES CMAKE TOOLS)
    set(var INSTALL_${type}_DIR)
    if (NOT IS_ABSOLUTE "${${var}}")
        set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
    endif ()
endforeach ()

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(cmake)
add_subdirectory(tools)

install(EXPORT ExportTargets
        NAMESPACE sancus_support_
        DESTINATION ${INSTALL_CMAKE_DIR})

set(CPACK_GENERATOR "DEB")

set(CPACK_PACKAGE_NAME "sancus-support")
set(CPACK_PACKAGE_VERSION "1.0")
set(CPACK_PACKAGE_CONTACT "Job Noorman <Job.Noorman@cs.kuleuven.be>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Sancus support tools")

#set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "python3, gcc-msp430, llvm-sancus")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE
        "https://distrinet.cs.kuleuven.be/software/sancus/")

include(CPack)
