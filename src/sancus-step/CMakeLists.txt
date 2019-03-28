if(SLLVM_INSTALL_DIR)

 set(SANCUS_SECURITY 64)
 set(STACK_SIZE 128)
 set(VENDOR_ID 0x1234)

 set(CLANG ${SLLVM_INSTALL_DIR}/bin/clang)
 set(OPT ${SLLVM_INSTALL_DIR}/bin/opt)
 set(LLC ${SLLVM_INSTALL_DIR}/bin/llc)
 set(AS ${SLLVM_INSTALL_DIR}/bin/msp430-elf-gcc)

 set(INCFLAGS
     -I ${SLLVM_INSTALL_DIR}/include
     -I ${SLLVM_INSTALL_DIR}/msp430-elf/include
     -I ${SLLVM_INSTALL_DIR}/share/sancus-compiler/include
     -I ${CMAKE_CURRENT_SOURCE_DIR}/../../include/sancus_support
 )

 set(CFLAGS
     ${INCFLAGS}
     -D__MSP430SANCUS__
#     -DSANCUS_SECURITY=${SANCUS_SECURITY}
     -O0
     -target msp430-elf
     -emit-llvm
 )

 set (SLLVM_TRANSFORM_FLAGS
      --load ${SLLVM_INSTALL_DIR}/lib/SLLVM.so
      --sllvm-transform
 )

 set (SANCUS_TRANSFORM_FLAGS
  --load ${SLLVM_INSTALL_DIR}/lib/SLLVM.so
  --sancus-transform
  --key-size ${SANCUS_SECURITY}
  --stack-size ${STACK_SIZE}
  --vendor-id ${VENDOR_ID}
 )

 set (LLCFLAGS
      -O3
 )

 set (ASFLAGS
     ${INCFLAGS}
     -mmcu=msp430sancus
 )

 set(OBJ
     sancus_step.o
 )

 add_custom_command(OUTPUT sancus_step.ll
  COMMAND ${CLANG} ${CFLAGS} -S ${CMAKE_CURRENT_SOURCE_DIR}/sancus_step.c -o sancus_step.ll
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sancus_step.c
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../../include/sancus_support/sancus_step.h
 )

 add_custom_command(OUTPUT sancus_step.sllvm.ll
  COMMAND ${OPT} ${SLLVM_TRANSFORM_FLAGS} -S sancus_step.ll -o sancus_step.sllvm.ll
  DEPENDS sancus_step.ll
 )

 add_custom_command(OUTPUT sancus_step.sancus.ll
  COMMAND ${OPT} ${SANCUS_TRANSFORM_FLAGS} -S sancus_step.sllvm.ll -o sancus_step.sancus.ll
  DEPENDS sancus_step.sllvm.ll
 )

 add_custom_command(OUTPUT sancus_step.s
  COMMAND ${LLC} ${LLCFLAGS} sancus_step.sancus.ll -o sancus_step.s
  DEPENDS sancus_step.sancus.ll
 )

 add_custom_command(OUTPUT sancus_step.o
  COMMAND ${AS} ${ASFLAGS} -c sancus_step.s -o sancus_step.o
  DEPENDS sancus_step.s
 )

 add_library(sancus-step STATIC ${OBJ})
 install(TARGETS sancus-step
         EXPORT ExportTargets
         ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})

 set_target_properties(sancus-step PROPERTIES LINKER_LANGUAGE C)

else()

 set(CMAKE_C_COMPILER sancus-cc)
 set(CMAKE_C_CREATE_STATIC_LIBRARY "sancus-ld --verbose <OBJECTS> -o <TARGET>")

 set(SRC
     sancus_step.c
 )

 include_directories(${SANCUS_INCLUDES})
 add_library(sancus-step STATIC ${SRC})
 install(TARGETS sancus-step
         EXPORT ExportTargets
         ARCHIVE DESTINATION ${INSTALL_LIBS_DIR})

endif()
