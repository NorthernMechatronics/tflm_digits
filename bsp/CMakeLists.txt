set(BSP_GENERATOR
    ${CMAKE_SOURCE_DIR}/tools/bsp_generator/nm_pinconfig.py
)

set(BSP_PINS_DIR ${CMAKE_CURRENT_LIST_DIR}/${BSP_TARGET_DIR})
set(BSP_PINS_SRC ${BSP_PINS_DIR}/bsp_pins.src)
set(BSP_PINS_H ${BSP_PINS_DIR}/am_bsp_pins.h)
set(BSP_PINS_C ${BSP_PINS_DIR}/am_bsp_pins.c)

add_custom_command(
    OUTPUT
        ${BSP_PINS_H}
        ${BSP_PINS_C}
    COMMAND
        python ${BSP_GENERATOR} ${BSP_PINS_SRC}
    WORKING_DIRECTORY
        ${BSP_PINS_DIR}
)

add_library(
    bsp
    STATIC
)

add_dependencies(bsp hal)

target_include_directories(
    bsp
    PRIVATE
    ${HAL_INCLUDES}
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/${BSP_TARGET_DIR}
)

target_sources(
    bsp PRIVATE
    am_bsp.c
    ${BSP_PINS_C}
    ${BSP_PINS_H}
)