cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.13.0)

get_filename_component(APPLICATION ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

set(TARGET "nm180100" CACHE STRING "" FORCE)
set(BSP_TARGET nm180100evb CACHE STRING "" FORCE)

option(RAT_BLE_ENABLE "" OFF)
option(RAT_LORAWAN_ENABLE "" OFF)

add_subdirectory(nmsdk2)

project(${APPLICATION})

add_executable(${APPLICATION})
add_subdirectory(bsp)

if (RAT_BLE_ENABLE)
    message("BLE enabled")
    set(BLE_DEFINES
        -DRAT_BLE_ENABLE
    )

    set(BLE_INCLUDES
        ${BLE_INCLUDES}
        ${PROJECT_SOURCE_DIR}/comms/ble
        ${PROJECT_SOURCE_DIR}/comms/ble/amota
        ${PROJECT_SOURCE_DIR}/comms/ble/amota/profile
        ${PROJECT_SOURCE_DIR}/comms/ble/amota/service
    )

    set(BLE_SOURCES
        comms/ble/ble_stack.c
        comms/ble/ble_task_cli.c
        comms/ble/ble_task.c
        comms/ble/amota/amota_main.c
        comms/ble/amota/profile/amotas_main.c
        comms/ble/amota/service/svc_amotas.c
    )

    set(BLE_LIBS
        ble
    )
endif()

if (RAT_LORAWAN_ENABLE)
    message("LoRaWAN enabled")
    set(LORAWAN_DEFINES
        -DRAT_LORAWAN_ENABLE
        -DSOFT_SE
        -DCONTEXT_MANAGEMENT_ENABLED
    )

    set(LORAWAN_INCLUDES
        ${LORAWAN_INCLUDES}
        ${PROJECT_SOURCE_DIR}/comms/lorawan
        ${PROJECT_SOURCE_DIR}/comms/lorawan/soft-se
    )

    set(LORAWAN_SOURCES
        comms/lorawan/lmh_callbacks.c
        comms/lorawan/lmhp_fragmentation.c
        comms/lorawan/lorawan_se.c
        comms/lorawan/lorawan_task_cli.c
        comms/lorawan/lorawan_task.c
        comms/lorawan/soft-se/aes.c
        comms/lorawan/soft-se/cmac.c
        comms/lorawan/soft-se/soft-se.c
    )

    set(LORAWAN_LIBS
        lorawan
    )
endif()

target_link_libraries(
    ${APPLICATION}
    PRIVATE
        ${BLE_LIBS}
        ${LORAWAN_LIBS}
        rtos
        bsp
        hal
    PUBLIC
        -T${PROJECT_SOURCE_DIR}/AMA3B1KK.ld
        -Wl,--start-group
        -lm
        -lc
        -lgcc
        -Wl,--end-group
        -Wl,--gc-sections
)

target_compile_definitions(
    ${APPLICATION}
    PRIVATE
    ${BLE_DEFINES}
    ${LORAWAN_DEFINES}
)

target_include_directories(
    ${APPLICATION}
    PRIVATE
    ${HAL_INCLUDES}
    ${RTOS_INCLUDES}
    ${BLE_INCLUDES}
    ${LORAWAN_INCLUDES}
    ${PROJECT_SOURCE_DIR}/bsp
    ${PROJECT_SOURCE_DIR}/bsp/${BSP_TARGET}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/config
    ${PROJECT_SOURCE_DIR}/utils/bootloader
    ${PROJECT_SOURCE_DIR}/utils/RTT/Config
    ${PROJECT_SOURCE_DIR}/utils/RTT/RTT
)

target_sources(
    ${APPLICATION}
    PRIVATE
    startup_gcc.c
    main.c
    application_task_cli.c
    application_task.c
    console_task.c

    utils/bootloader/am_bootloader.c
    utils/bootloader/am_multi_boot.c

    utils/RTT/RTT/SEGGER_RTT.c
    utils/RTT/RTT/SEGGER_RTT_printf.c

    ${BLE_SOURCES}
    ${LORAWAN_SOURCES}
)

add_custom_command(
    TARGET ${APPLICATION}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE_NAME:${APPLICATION}> $<TARGET_FILE_NAME:${APPLICATION}>.bin
)

add_dependencies(${APPLICATION} hal bsp rtos ${BLE_LIBS} ${LORAWAN_LIBS})