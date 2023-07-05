set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

set(TARGET_TRIPLET "arm-none-eabi-")
set(CMAKE_C_COMPILER   ${TARGET_TRIPLET}gcc${TOOLCHAIN_EXT})
set(CMAKE_CXX_COMPILER ${TARGET_TRIPLET}g++${TOOLCHAIN_EXT})
set(CMAKE_ASM_COMPILER ${TARGET_TRIPLET}as${TOOLCHAIN_EXT})
set(CMAKE_LINKER       ${TARGET_TRIPLET}gcc${TOOLCHAIN_EXT})
set(CMAKE_SIZE_UTIL    ${TARGET_TRIPLET}size${TOOLCHAIN_EXT})
set(CMAKE_OBJCOPY      ${TARGET_TRIPLET}objcopy${TOOLCHAIN_EXT})
set(CMAKE_OBJDUMP      ${TARGET_TRIPLET}objdump${TOOLCHAIN_EXT})
set(CMAKE_NM_UTIL      ${TARGET_TRIPLET}gcc-nm${TOOLCHAIN_EXT})
set(CMAKE_AR           ${TARGET_TRIPLET}gcc-ar${TOOLCHAIN_EXT})
set(CMAKE_RANLIB       ${TARGET_TRIPLET}gcc-ranlib${TOOLCHAIN_EXT})

set(CMAKE_C_STANDARD   99)
set(CMAKE_CXX_STANDARD 11)

set(COMMON_C_FLAGS              "-mthumb -fdata-sections -ffunction-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS               "${COMMON_C_FLAGS}")
set(CMAKE_CXX_FLAGS             "${COMMON_C_FLAGS} -fno-exceptions -fno-rtti")
set(CMAKE_ASM_FLAGS             "${COMMON_C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${COMMON_C_FLAGS} --specs=nano.specs --specs=nosys.specs")

set(CMAKE_C_FLAGS_DEBUG         "-g -O0")
set(CMAKE_C_FLAGS_RELEASE       "-Os")

add_compile_definitions(
    gcc
    PART_apollo3
    AM_PART_APOLLO3
    AM_PACKAGE_BGA
)

add_compile_definitions(
    $<$<CONFIG:debug>:AM_ASSERT_INVALID_THRESHOLD=0>
)

add_compile_definitions(
    $<$<CONFIG:debug>:AM_DEBUG_ASSERT>
)

add_compile_definitions(
    $<$<CONFIG:debug>:AM_DEBUG_PRINTF>
)