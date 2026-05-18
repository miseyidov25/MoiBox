IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=soft")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE=1")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_DEBUG " \
    ${CMAKE_ASM_FLAGS_DEBUG} \
    -DCPU_MCXA153VLH \
    -DMCXA153_SERIES \
    -D__STARTUP_CLEAR_BSS \
    -DMCUXPRESSO_SDK \
    -include \
    ${ProjDirPath}/config/mcux_config.h \
    -include \
    ${ProjDirPath}/config/mcuxsdk_version.h \
    -mthumb \
    -mcpu=cortex-m33+nodsp \
    -g \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_DEBUG " \
    ${CMAKE_C_FLAGS_DEBUG} \
    -DCPU_MCXA153VLH \
    -DMCXA153_SERIES \
    -D__STARTUP_CLEAR_BSS \
    -DDEBUG \
    -DMCUX_META_BUILD \
    -DMCUXPRESSO_SDK \
    -include \
    ${ProjDirPath}/config/mcux_config.h \
    -include \
    ${ProjDirPath}/config/mcuxsdk_version.h \
    --specs=nano.specs \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -std=gnu99 \
    -Werror \
    -fstack-usage \
    -mcpu=cortex-m33+nodsp \
    -g \
    -O0 \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_DEBUG} \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -Wl,--gc-sections \
    -Wl,-static \
    -Wl,--print-memory-usage \
    -Xlinker \
    -Map=output.map \
    -mcpu=cortex-m33+nodsp \
    -g \
    -Wl,--no-warn-rwx-segments \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/config/devices/MCX/MCXA/MCXA153/gcc/MCXA153_flash.ld\" -static \
")
