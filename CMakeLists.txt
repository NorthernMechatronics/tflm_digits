cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.13.0)

get_filename_component(APPLICATION ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

set(TARGET "nm180100" CACHE STRING "" FORCE)
set(BSP_TARGET nm180100evb CACHE STRING "" FORCE)

add_subdirectory(nmsdk2)

project(${APPLICATION})

add_executable(${APPLICATION})
add_subdirectory(bsp)

target_link_libraries(
    ${APPLICATION}
    PRIVATE
        rtos
        bsp
        hal
    PUBLIC
#        -T${PROJECT_SOURCE_DIR}/AMA3B1KK.ld
        -T${TARGET_LINK_SCRIPT}
        -Wl,--start-group
        -lstdc++
        -lsupc++
        -lm
        -lc
        -lgcc
        -Wl,--end-group
        -Wl,--gc-sections
)

target_compile_definitions(
    ${APPLICATION}
    PRIVATE
)

target_include_directories(
    ${APPLICATION}
    PRIVATE
    ${HAL_INCLUDES}
    ${RTOS_INCLUDES}
    ${PROJECT_SOURCE_DIR}/bsp
    ${PROJECT_SOURCE_DIR}/bsp/${BSP_TARGET}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/config
    ${PROJECT_SOURCE_DIR}/utils/RTT/Config
    ${PROJECT_SOURCE_DIR}/utils/RTT/RTT
)

target_sources(
    ${APPLICATION}
    PRIVATE
    ${TARGET_SRC}
    main.c
    application_task_cli.c
    application_task.c
    console_task.c

    utils/RTT/RTT/SEGGER_RTT.c
    utils/RTT/RTT/SEGGER_RTT_printf.c
)

add_custom_command(
    TARGET ${APPLICATION}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE_NAME:${APPLICATION}> $<TARGET_FILE_NAME:${APPLICATION}>.bin
)

add_dependencies(${APPLICATION} hal bsp rtos)