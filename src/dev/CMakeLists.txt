add_library(obj-timer OBJECT timer.c)
add_library(obj-uart  OBJECT uart.c)
add_library(obj-ps2   OBJECT ps2.c)

add_library(dev-timer STATIC $<TARGET_OBJECTS:obj-timer>)
add_library(dev-uart  STATIC $<TARGET_OBJECTS:obj-uart>)
add_library(dev-ps2   STATIC $<TARGET_OBJECTS:obj-ps2>)

add_library(dev STATIC $<TARGET_OBJECTS:obj-timer>
                       $<TARGET_OBJECTS:obj-uart>
                       $<TARGET_OBJECTS:obj-ps2>)

install(TARGETS dev-timer dev-uart dev-ps2 dev
        EXPORT ExportTargets
        ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
