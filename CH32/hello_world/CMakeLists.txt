cmake_minimum_required(VERSION 3.15)

# Toolchain file must be set BEFORE project() — CMake locks in the
# compiler during project(), so setting it later has no effect.
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain.cmake)
endif()

project(CH32_HelloWorld C ASM)

# Compiler and target settings
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR RISCV)
set(CMAKE_CROSSCOMPILING TRUE)

# Optimization flags
set(CMAKE_C_FLAGS_INIT "-march=rv32ec -mabi=ilp32e -msmall-data-limit=0 -msave-restore")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_INIT} -ffunction-sections -fdata-sections -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g -fdiagnostics-color=auto")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS_INIT} -x assembler-with-cpp")

# Linker flags
# -nostartfiles: don't link the toolchain's default crt0.o — it defines its
# own _start and expects __bss_start/__bss_end (newlib convention), which
# conflicts with startup_ch32v00x.S and Link.ld (which use _sbss/_ebss).
# --specs=nano.specs: link the small newlib-nano libc instead of full newlib
# (full libc's printf doesn't fit in this chip's 16K flash).
# --specs=nosys.specs: provide stub syscalls (_sbrk, _close, _lseek, etc.)
# that newlib needs but debug.c doesn't implement (it only provides _write).
set(CMAKE_EXE_LINKER_FLAGS "-nostartfiles --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections -Wl,-Map=output.map")

# WCH CH32-SDK (EVT) — fetched directly from WCH's own domain rather than
# vendored, since its files carry WCH's own usage notice (see README.md).
include(FetchContent)
FetchContent_Declare(
    ch32_evt
    URL https://file.wch.cn/download/file?id=412
    URL_HASH SHA256=8a1d5bbb2117b3847aa2807ea238b6039a378b95ce5a838feed069707198933d
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(ch32_evt)

# Include paths
# Note: FetchContent's archive extraction strips the zip's single top-level
# "EVT/" directory, so the fetched tree lands at EXAM/SRC, not EVT/EXAM/SRC.
set(SDK_PATH ${ch32_evt_SOURCE_DIR}/EXAM/SRC)
set(PERIPH_INC ${SDK_PATH}/Peripheral/inc)
set(CORE_INC ${SDK_PATH}/Core)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PERIPH_INC}
    ${CORE_INC}
)

# SDK Source files
file(GLOB PERIPH_SOURCES ${SDK_PATH}/Peripheral/src/*.c)
set(CORE_SOURCES ${SDK_PATH}/Core/core_riscv.c)
set(STARTUP_SOURCE ${SDK_PATH}/Startup/startup_ch32v00x.S)

# Project source files
set(PROJECT_SOURCES
    src/main.c
    src/system_ch32v00x.c
    src/ch32v00x_it.c
    src/debug.c
)

# Create executable
add_executable(${PROJECT_NAME}.elf
    ${PROJECT_SOURCES}
    ${PERIPH_SOURCES}
    ${CORE_SOURCES}
    ${STARTUP_SOURCE}
)

# Link linker script
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${CMAKE_CURRENT_SOURCE_DIR}/Link.ld
)

# Generate hex and bin files
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMENT "Building hex and bin files"
)

# Print build info
message(STATUS "CH32 Project: ${PROJECT_NAME}")
message(STATUS "SDK Path: ${SDK_PATH}")
message(STATUS "Toolchain: RISC-V Embedded GCC")
