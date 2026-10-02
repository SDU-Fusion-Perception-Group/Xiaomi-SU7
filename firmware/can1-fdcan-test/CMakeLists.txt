cmake_minimum_required(VERSION 3.22)
project(can1_fdcan_test C ASM)
set(CMAKE_C_STANDARD 11)
if(NOT DEFINED OFFICIAL_CAN_ROOT)
    set(OFFICIAL_CAN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../dm-mc02/例程/CtrBoard-H7_CAN")
endif()
add_executable(can1-fdcan-test
    "${OFFICIAL_CAN_ROOT}/Core/Src/main.c"
    "${OFFICIAL_CAN_ROOT}/Core/Src/gpio.c"
    "${OFFICIAL_CAN_ROOT}/Core/Src/fdcan.c"
    "${OFFICIAL_CAN_ROOT}/Core/Src/stm32h7xx_it.c"
    "${OFFICIAL_CAN_ROOT}/Core/Src/stm32h7xx_hal_msp.c"
    "${OFFICIAL_CAN_ROOT}/Core/Src/system_stm32h7xx.c"
    "${OFFICIAL_CAN_ROOT}/User/bsp_fdcan.c"
    "C:/Users/S/dmc-build/startup/startup_stm32h723xx.s"
    "C:/Users/S/dmc-build/startup/system_boot_compat.c"
    classic_override.c)
foreach(name hal hal_cortex hal_rcc hal_rcc_ex hal_gpio hal_pwr hal_pwr_ex hal_flash hal_flash_ex hal_dma hal_dma_ex hal_fdcan)
    target_sources(can1-fdcan-test PRIVATE "${OFFICIAL_CAN_ROOT}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_${name}.c")
endforeach()
target_include_directories(can1-fdcan-test PRIVATE
    "${OFFICIAL_CAN_ROOT}/Core/Inc" "${OFFICIAL_CAN_ROOT}/User"
    "${OFFICIAL_CAN_ROOT}/Drivers/STM32H7xx_HAL_Driver/Inc"
    "${OFFICIAL_CAN_ROOT}/Drivers/CMSIS/Device/ST/STM32H7xx/Include"
    "${OFFICIAL_CAN_ROOT}/Drivers/CMSIS/Include")
target_compile_definitions(can1-fdcan-test PRIVATE STM32H723xx USE_PWR_LDO_SUPPLY USE_HAL_DRIVER)
target_compile_options(can1-fdcan-test PRIVATE -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -Wall -Wno-unused-parameter)
target_link_options(can1-fdcan-test PRIVATE -Wl,--allow-multiple-definition)
target_link_options(can1-fdcan-test PRIVATE -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -TC:/Users/S/dmc-build/startup/STM32H723VGTx_FLASH.ld -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs)
add_custom_command(TARGET can1-fdcan-test POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:can1-fdcan-test> ${CMAKE_CURRENT_BINARY_DIR}/can1-fdcan-test.hex)
