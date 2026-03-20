cmake_minimum_required(VERSION 3.20)

project(template LANGUAGES C ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(USER_DIR "${ROOT_DIR}/user")
set(COMMON_DIR "${ROOT_DIR}/common")
set(CONFIG_DIR "${ROOT_DIR}/config")
set(PLATFORM_DIR "${ROOT_DIR}/platform")
set(BOARD_DIR "${ROOT_DIR}/board/custom")
set(DRIVERS_DIR "${ROOT_DIR}/drivers")
set(SERVICES_DIR "${ROOT_DIR}/services")
set(APP_DIR "${ROOT_DIR}/app")
set(LIB_DIR "${ROOT_DIR}/Libraries")
set(LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/STM32F103XB_FLASH.ld")
set(STARTUP_FILE "${LIB_DIR}/startup/startup_stm32f103xb.s")
set(MCU_FLAGS
    -mcpu=cortex-m3
    -mthumb
)

set(VENDOR_SOURCES
    "${LIB_DIR}/hal/stm32f1xx_hal.c"
    "${LIB_DIR}/hal/stm32f1xx_hal_cortex.c"
    "${LIB_DIR}/hal/stm32f1xx_hal_flash.c"
    "${LIB_DIR}/hal/stm32f1xx_hal_gpio.c"
    "${LIB_DIR}/hal/stm32f1xx_hal_rcc.c"
    "${LIB_DIR}/hal/stm32f1xx_hal_rcc_ex.c"
    "${LIB_DIR}/stm32f1/system_stm32f1xx.c"
    "${STARTUP_FILE}"
)

set(COMMON_SOURCES
    "${COMMON_DIR}/project_assert.c"
)

set(PLATFORM_SOURCES
    "${PLATFORM_DIR}/platform.c"
    "${PLATFORM_DIR}/platform_irq.c"
    "${PLATFORM_DIR}/platform_msp.c"
)

set(BOARD_SOURCES
    "${BOARD_DIR}/board.c"
)

set(DRIVER_SOURCES
    "${DRIVERS_DIR}/drivers.c"
)

set(SERVICE_SOURCES
    "${SERVICES_DIR}/services.c"
)

set(APP_SOURCES
    "${APP_DIR}/app.c"
)

set(USER_SOURCES
    "${USER_DIR}/main.c"
    "${USER_DIR}/runtime_stubs.c"
    "${USER_DIR}/stm32f1xx_hal_msp.c"
    "${USER_DIR}/stm32f1xx_it.c"
)

set(SOURCE_FILES
    ${VENDOR_SOURCES}
    ${COMMON_SOURCES}
    ${PLATFORM_SOURCES}
    ${BOARD_SOURCES}
    ${DRIVER_SOURCES}
    ${SERVICE_SOURCES}
    ${APP_SOURCES}
    ${USER_SOURCES}
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".elf"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${APP_DIR}"
    "${BOARD_DIR}"
    "${COMMON_DIR}"
    "${CONFIG_DIR}"
    "${CONFIG_DIR}/board"
    "${CONFIG_DIR}/product"
    "${CONFIG_DIR}/target"
    "${DRIVERS_DIR}"
    "${PLATFORM_DIR}"
    "${SERVICES_DIR}"
    "${USER_DIR}"
    "${LIB_DIR}/cmsis"
    "${LIB_DIR}/hal"
    "${LIB_DIR}/stm32f1"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    USE_HAL_DRIVER
    STM32F103xB
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${MCU_FLAGS}
    -ffunction-sections
    -fdata-sections
    -Wall
    -Wextra
    -g3
)

target_link_options(${PROJECT_NAME} PRIVATE
    ${MCU_FLAGS}
    -T${LINKER_SCRIPT}
    -nostartfiles
    -static
    -Wl,--gc-sections
    -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map
    -Wl,--print-memory-usage
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    c
    m
    gcc
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}>
)
