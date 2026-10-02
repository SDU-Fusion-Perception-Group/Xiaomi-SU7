cmake_minimum_required(VERSION 3.22)
project(dm_mc02_module_bench C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
if(NOT DEFINED OFFICIAL_ROOT)
    set(OFFICIAL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../dm-mc02/例程/CtrBoard-H7_ALL")
endif()

add_executable(module-bench
    App/module_bench.c
    App/command_shell.c
    App/chassis_motion.c
    Bsp/rs485_port.c
    Bsp/can3_port.c
    Bsp/bmi088_dma.c
    Bsp/status_indicator.c
    Protocol/modbus_rtu.c
    Protocol/lyx_motor.c
    Protocol/dji_3508.c
    Protocol/protocol_store.c
    "${OFFICIAL_ROOT}/Bsp/w25q64.c"
    Telemetry/usb_telemetry.c
    Telemetry/mavlink_min.c
    "${OFFICIAL_ROOT}/Core/Src/main.c"
    "${OFFICIAL_ROOT}/Core/Src/gpio.c"
    "${OFFICIAL_ROOT}/Core/Src/freertos.c"
    "${OFFICIAL_ROOT}/Core/Src/adc.c"
    "${OFFICIAL_ROOT}/Core/Src/dma.c"
    "${OFFICIAL_ROOT}/Core/Src/fdcan.c"
    "${OFFICIAL_ROOT}/Core/Src/spi.c"
    "${OFFICIAL_ROOT}/Core/Src/tim.c"
    "${OFFICIAL_ROOT}/Core/Src/usart.c"
    "${OFFICIAL_ROOT}/Core/Src/stm32h7xx_it.c"
    "${OFFICIAL_ROOT}/Core/Src/stm32h7xx_hal_msp.c"
    "${OFFICIAL_ROOT}/Core/Src/stm32h7xx_hal_timebase_tim.c"
    "${OFFICIAL_ROOT}/Core/Src/system_stm32h7xx.c"
    "${OFFICIAL_ROOT}/Core/Src/octospi.c"
    startup/system_boot_compat.c
    "${OFFICIAL_ROOT}/USB_DEVICE/App/usb_device.c"
    "${OFFICIAL_ROOT}/USB_DEVICE/App/usbd_desc.c"
    "${OFFICIAL_ROOT}/USB_DEVICE/App/usbd_cdc_if.c"
    "${OFFICIAL_ROOT}/USB_DEVICE/Target/usbd_conf.c"
    "${OFFICIAL_ROOT}/Bsp/bsp_buzzer.c"
    "${OFFICIAL_ROOT}/Bsp/ws2812.c"
    "${OFFICIAL_ROOT}/Bsp/BMI088driver.c"
    "${OFFICIAL_ROOT}/Bsp/BMI088Middleware.c"
    "${OFFICIAL_ROOT}/Algorithm/MahonyAHRS.c"
    startup/startup_stm32h723xx.s
)

file(GLOB HAL_SOURCES CONFIGURE_DEPENDS
    "${OFFICIAL_ROOT}/Drivers/STM32H7xx_HAL_Driver/Src/*.c")
file(GLOB FREERTOS_SOURCES CONFIGURE_DEPENDS
    "${OFFICIAL_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/*.c")
target_sources(module-bench PRIVATE
    ${HAL_SOURCES}
    ${FREERTOS_SOURCES}
    "${OFFICIAL_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c"
    "${OFFICIAL_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/GCC/ARM_CM7_R0P1/port.c"
    "${OFFICIAL_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c"
    "${OFFICIAL_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c"
    "${OFFICIAL_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c"
    "${OFFICIAL_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c"
)

target_include_directories(module-bench PRIVATE
    App Bsp Protocol Telemetry
    "${OFFICIAL_ROOT}/Core/Inc"
    "${OFFICIAL_ROOT}/USB_DEVICE/App"
    "${OFFICIAL_ROOT}/USB_DEVICE/Target"
    "${OFFICIAL_ROOT}/Bsp/inc"
    "${OFFICIAL_ROOT}/Algorithm/inc"
    "${OFFICIAL_ROOT}/vofa+"
    "${OFFICIAL_ROOT}/Drivers/STM32H7xx_HAL_Driver/Inc"
    "${OFFICIAL_ROOT}/Drivers/CMSIS/Device/ST/STM32H7xx/Include"
    "${OFFICIAL_ROOT}/Drivers/CMSIS/Include"
    "${OFFICIAL_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/include"
    "${OFFICIAL_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/GCC/ARM_CM7_R0P1"
    "${OFFICIAL_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc"
    "${OFFICIAL_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc"
)

target_compile_definitions(module-bench PRIVATE
    STM32H723xx USE_PWR_LDO_SUPPLY USE_HAL_DRIVER)
target_compile_options(module-bench PRIVATE
    -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
    -std=gnu11 -ffunction-sections -fdata-sections
    -Wall -Wextra -Wno-unused-parameter)
target_link_options(module-bench PRIVATE
    -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
    -T${CMAKE_CURRENT_SOURCE_DIR}/startup/STM32H723VGTx_FLASH.ld
    -Wl,--gc-sections -Wl,-Map=$<TARGET_FILE_DIR:module-bench>/module-bench.map
    --specs=nano.specs --specs=nosys.specs -u _printf_float
    -Wl,--no-warn-rwx-segments)
target_link_libraries(module-bench PRIVATE m)

add_custom_command(TARGET module-bench POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:module-bench> module-bench.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:module-bench>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM)
