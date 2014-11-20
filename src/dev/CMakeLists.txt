add_library(obj-timer    OBJECT timer.c)
add_library(obj-uart     OBJECT uart.c)
add_library(obj-ps2      OBJECT ps2.c)
add_library(obj-pmodcls  OBJECT pmodcls.c)
add_library(obj-pmodkypd OBJECT pmodkypd.c)
add_library(obj-spi      OBJECT spi.c)

add_library(dev-timer    STATIC $<TARGET_OBJECTS:obj-timer>)
add_library(dev-uart     STATIC $<TARGET_OBJECTS:obj-uart>)
add_library(dev-ps2      STATIC $<TARGET_OBJECTS:obj-ps2>)
add_library(dev-pmodcls  STATIC $<TARGET_OBJECTS:obj-pmodcls>)
add_library(dev-pmodkypd STATIC $<TARGET_OBJECTS:obj-pmodkypd>)
add_library(dev-spi      STATIC $<TARGET_OBJECTS:obj-spi>)

add_library(dev STATIC $<TARGET_OBJECTS:obj-timer>
                       $<TARGET_OBJECTS:obj-uart>
                       $<TARGET_OBJECTS:obj-ps2>
                       $<TARGET_OBJECTS:obj-pmodcls>
                       $<TARGET_OBJECTS:obj-pmodkypd>
                       $<TARGET_OBJECTS:obj-spi>)

install(TARGETS dev-timer dev-uart dev-ps2 dev-pmodcls dev-pmodkypd dev-spi dev
        EXPORT ExportTargets
        ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})
