cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.13.0)

get_filename_component(APPLICATION ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

if (NOT "$ENV{APPLICATION_NAME}" STREQUAL "")
    set(APPLICATION_NAME $ENV{APPLICATION_NAME})
else()
    set(APPLICATION_NAME ${APPLICATION})
endif()

set(TARGET "nm180100" CACHE STRING "" FORCE)

option(BSP_NM180100EVB "" OFF)
option(BSP_NM180310 "" OFF)
option(BSP_NM180410 "" OFF)

option(RAT_BLE_ENABLE "" OFF)
option(RAT_LORAWAN_ENABLE "" OFF)
option(TF_ENABLE "" OFF)

option(MODEL_SIZE_SMALL "" OFF)
option(MODEL_SIZE_MEDIUM "" OFF)
option(MODEL_SIZE_LARGE "" OFF)

if (BSP_NM180100EVB)
add_definitions(-DBSP_NM180100EVB)
set(BSP_TARGET_DIR nm180100evb CACHE STRING "" FORCE)
endif()

if (BSP_NM180310)
add_definitions(-DBSP_NM180310)
set(BSP_TARGET_DIR nm180310 CACHE STRING "" FORCE)
endif()

if (BSP_NM180410)
add_definitions(-DBSP_NM180410)
set(BSP_TARGET_DIR nm180410 CACHE STRING "" FORCE)
endif()

if (MODEL_SIZE_SMALL)
    add_definitions(-DMODEL_SIZE_SMALL)
    set(MODEL_SRC quant_model_small.cc CACHE STRING "" FORCE)
endif()

if (MODEL_SIZE_MEDIUM)
    add_definitions(-DMODEL_SIZE_MEDIUM)
    set(MODEL_SRC quant_model_medium.cc CACHE STRING "" FORCE)
endif()

if (MODEL_SIZE_LARGE)
    add_definitions(-DMODEL_SIZE_LARGE)
    set(MODEL_SRC quant_model_large.cc CACHE STRING "" FORCE)
endif()

add_subdirectory(nmsdk2)

project(${APPLICATION})

add_executable(${APPLICATION})
set_target_properties(
    ${APPLICATION}
    PROPERTIES
        OUTPUT_NAME ${APPLICATION_NAME}
        SUFFIX ".axf"
)

add_subdirectory(bsp)

target_link_libraries(
    ${APPLICATION}
    PRIVATE
        rtos
        bsp
        hal
        tflm
    PUBLIC
        -T${TARGET_LINK_SCRIPT}
        -Wl,--start-group
        -L${CMSIS_LIB_DIR}
        -lstdc++
        -lsupc++
        -lm
        -lc
        -lgcc
        -lCMSISDSP
        -lcmsis-nn
        -Wl,--end-group
        -Wl,--gc-sections
)

target_compile_definitions(
    ${APPLICATION}
    PRIVATE
	-DGEMMLOWP_ALLOW_SLOW_SCALAR_FALLBACK
    -DTF_LITE_STATIC_MEMORY
    -DTF_LITE_MCU_DEBUG_LOG
    -DARM_MATH_CM4
)

target_include_directories(
    ${APPLICATION}
    PRIVATE
    ${HAL_INCLUDES}
    ${RTOS_INCLUDES}
    ${TFLM_INCLUDES}

    ${PROJECT_SOURCE_DIR}/bsp
    ${PROJECT_SOURCE_DIR}/bsp/${BSP_TARGET_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/config
    ${PROJECT_SOURCE_DIR}/utils/RTT/Config
    ${PROJECT_SOURCE_DIR}/utils/RTT/RTT

    ${PROJECT_SOURCE_DIR}/drivers/arducam
)

target_sources(
    ${APPLICATION}
    PRIVATE
    ${TARGET_SRC}
    main.c
    application_task_cli.c
    application_task.c
    button_task.c
    camera_task.c
    camera_task_cli.c
    console_task.c
    stub.c

    drivers/arducam/ArducamAmbiqHAL.c
    drivers/arducam/ArducamCamera.c
    drivers/arducam/ArducamLink.c
    drivers/arducam/ArducamUart.c

    constants.cc
    model_settings.cc
    ${MODEL_SRC}
    output_handler.cc
    tflm.cc

    utils/RTT/RTT/SEGGER_RTT.c
    utils/RTT/RTT/SEGGER_RTT_printf.c
)

add_custom_command(
    TARGET ${APPLICATION}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE_NAME:${APPLICATION}> $<TARGET_FILE_NAME:${APPLICATION}>.bin
)

add_dependencies(${APPLICATION} hal bsp rtos tflm)