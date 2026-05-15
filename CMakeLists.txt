# CROSS COMPILER SETTING
set(CMAKE_SYSTEM_NAME Generic)
cmake_minimum_required(VERSION 3.10.0)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${CMAKE_CURRENT_LIST_DIR})
endif()

include(${CMAKE_CURRENT_LIST_DIR}/config/config.cmake)
include(${SdkRootDirPath}/cmake/toolchain/toolchain.cmake)

# THE VERSION NUMBER
SET (MCUXPRESSO_CMAKE_FORMAT_MAJOR_VERSION 2)
SET (MCUXPRESSO_CMAKE_FORMAT_MINOR_VERSION 0)

include(ide_overrides.cmake OPTIONAL)

if(CMAKE_SCRIPT_MODE_FILE)
  message("${MCUXPRESSO_CMAKE_FORMAT_MAJOR_VERSION}")
  return()
endif()


set(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
set(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

# CURRENT DIRECTORY
set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
set(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(program)

enable_language(ASM)

set(MCUX_BUILD_TYPES debug)

set(EXECUTABLE_OUTPUT_PATH ${ProjDirPath})
set(LIBRARY_OUTPUT_PATH ${ProjDirPath})
set(MCUX_SDK_PROJECT_NAME program.elf)

include(${ProjDirPath}/config/flags.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/config/devices/MCX/MCXA/MCXA153/system_MCXA153.c"
"${ProjDirPath}/config/devices/MCX/MCXA/MCXA153/gcc/startup_MCXA153.S"
"${ProjDirPath}/config/components/misc_utilities/fsl_syscall_stub.c"
"${ProjDirPath}/config/components/misc_utilities/fsl_sbrk.c"
"${ProjDirPath}/config/components/misc_utilities/fsl_memcpy.S"
"${ProjDirPath}/config/mcux_config.h"
"${ProjDirPath}/config/mcuxsdk_version.h"
"${ProjDirPath}/config/devices/MCX/MCXA/MCXA153/fsl_device_registers.h"
"${ProjDirPath}/config/devices/MCX/MCXA/MCXA153/MCXA153.h"
"${ProjDirPath}/config/devices/MCX/MCXA/MCXA153/MCXA153_features.h"
"${ProjDirPath}/config/devices/MCX/MCXA/MCXA153/MCXA153_COMMON.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_ADC.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_AOI.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_CDOG.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_CMC.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_CRC.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_CTIMER.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_DEBUGMAILBOX.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_DMA.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_EIM.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_EQDC.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_ERM.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_FMC.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_FMU.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_FMUTEST.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_FREQME.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_GLIKEY.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_GPIO.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_I3C.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_INPUTMUX.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_LPCMP.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_LPI2C.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_LPSPI.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_LPTMR.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_LPUART.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_MRCC.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_OSTIMER.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_PORT.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_PWM.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_SCG.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_SPC.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_SYSCON.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_TRDC.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_USB.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_UTICK.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_VBAT.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_WAKETIMER.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_WUU.h"
"${ProjDirPath}/config/devices/MCX/MCXA/periph/PERI_WWDT.h"
"${ProjDirPath}/config/devices/MCX/MCXA/MCXA153/system_MCXA153.h"
"${ProjDirPath}/config/arch/arm/CMSIS/Core/Include/cmsis_compiler.h"
"${ProjDirPath}/config/arch/arm/CMSIS/Core/Include/cmsis_version.h"
"${ProjDirPath}/config/arch/arm/CMSIS/Core/Include/cmsis_gcc.h"
"${ProjDirPath}/config/arch/arm/CMSIS/Core/Include/m-profile/cmsis_gcc_m.h"
"${ProjDirPath}/config/arch/arm/CMSIS/Core/Include/core_cm33.h"
"${ProjDirPath}/config/arch/arm/CMSIS/Core/Include/m-profile/armv8m_mpu.h"
"${ProjDirPath}/config/arch/arm/CMSIS/Core/Include/tz_context.h"
# Add more library entries here
"${ProjDirPath}/default_lib/serial/serial.c"
"${ProjDirPath}/default_lib/serial/retarget.c"
"${ProjDirPath}/default_lib/fifo/fifo.c"
"${ProjDirPath}/HAL/hal.c"
"${ProjDirPath}/HAL/Actuators/lock.c"
"${ProjDirPath}/HAL/BT/bt.c"
"${ProjDirPath}/HAL/Display/leds.c"
"${ProjDirPath}/HAL/Display/oled.c"
"${ProjDirPath}/HAL/Input/buttons.c"
"${ProjDirPath}/HAL/Input/keypad.c"
# Using HAL switches implementation (provide stubs in HAL if needed)
# "${ProjDirPath}/HAL/Input/switches.c"
# Using updated puzzles_handler.c instead of old puzzle.c
# "${ProjDirPath}/Puzzles/puzzle.c"
"${ProjDirPath}/Puzzles/puzzles_handler.c"
# Using HAL/BT implementation; avoid duplicate sources/bt.c
# "${ProjDirPath}/sources/bt.c"
"${ProjDirPath}/sources/buzzer.c"
# Keep sources/main.c and puzzle_box.c
"${ProjDirPath}/sources/main.c"
"${ProjDirPath}/sources/puzzle_box.c"
# Use sources switches implementation instead of HAL stub
"${ProjDirPath}/sources/switches.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/config/devices/MCX/MCXA/MCXA153
    ${ProjDirPath}/config/devices/MCX/MCXA/periph
    ${ProjDirPath}/config/arch/arm/CMSIS/Core/Include
    ${ProjDirPath}/config/arch/arm/CMSIS/Core/Include/m-profile
    ${ProjDirPath}/default_lib/board
    ${ProjDirPath}/default_lib/leds
    ${ProjDirPath}/default_lib/serial
    ${ProjDirPath}/default_lib/fifo
    ${ProjDirPath}/headers
    ${ProjDirPath}/HAL
    ${ProjDirPath}/Puzzles
    ${ProjDirPath}/sources
    #  Add more libraries libraries here if
)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/program.bin)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map;${EXECUTABLE_OUTPUT_PATH}/program.bin")

