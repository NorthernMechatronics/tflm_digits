add_library(hal STATIC)

set(HAL_DIR ${PROJECT_SOURCE_DIR}/hal/ambiq)

set (HAL_INCLUDES
    ${HAL_DIR}/CMSIS/AmbiqMicro/Include
    ${HAL_DIR}/CMSIS/ARM/Include
    ${HAL_DIR}/mcu
    ${HAL_DIR}/mcu/apollo3
    ${HAL_DIR}/mcu/apollo3/hal
    ${HAL_DIR}/mcu/apollo3/regs
    ${HAL_DIR}/utils
    CACHE PATH "HAL includes"
)

target_include_directories(
    hal PRIVATE
    ${HAL_INCLUDES}
)

target_sources(
    hal PRIVATE 
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_adc.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_ble.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_ble_patch.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_ble_patch_b0.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_burst.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_cachectrl.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_clkgen.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_cmdq.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_ctimer.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_debug.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_entropy.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_flash.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_global.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_gpio.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_interrupt.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_iom.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_ios.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_itm.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_mcuctrl.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_mspi.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_pdm.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_pwrctrl.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_queue.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_reset.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_rtc.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_scard.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_secure_ota.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_security.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_stimer.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_sysctrl.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_systick.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_tpiu.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_uart.c
    ${HAL_DIR}/mcu/apollo3/hal/am_hal_wdt.c

    ${HAL_DIR}/utils/am_util_ble.c
    ${HAL_DIR}/utils/am_util_debug.c
    ${HAL_DIR}/utils/am_util_delay.c
    ${HAL_DIR}/utils/am_util_faultisr.c
    ${HAL_DIR}/utils/am_util_id.c
    ${HAL_DIR}/utils/am_util_regdump.c
    ${HAL_DIR}/utils/am_util_stdio.c
    ${HAL_DIR}/utils/am_util_string.c
    ${HAL_DIR}/utils/am_util_time.c
)

install (TARGETS hal DESTINATION ${PROJECT_SOURCE_DIR}/lib/${TARGET})