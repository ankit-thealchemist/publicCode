# RISC-V Toolchain for CH32V00x
#
# Requires the MounRiver "RISC-V Embedded GCC" toolchain (riscv32-wch-elf-*).
# Point CH32_TOOLCHAIN_PATH at its root (the folder containing bin/riscv32-wch-elf-gcc),
# either as a CMake cache variable (-DCH32_TOOLCHAIN_PATH=...) or an environment variable.
# See README.md for where to download it.
set(CH32_TOOLCHAIN_PATH "" CACHE PATH "Path to the MounRiver RISC-V Embedded GCC toolchain root")
if(NOT CH32_TOOLCHAIN_PATH AND DEFINED ENV{CH32_TOOLCHAIN_PATH})
    set(CH32_TOOLCHAIN_PATH $ENV{CH32_TOOLCHAIN_PATH} CACHE PATH "Path to the MounRiver RISC-V Embedded GCC toolchain root" FORCE)
endif()
if(NOT CH32_TOOLCHAIN_PATH)
    message(FATAL_ERROR "CH32_TOOLCHAIN_PATH is not set. Pass -DCH32_TOOLCHAIN_PATH=/path/to/toolchain, or set the CH32_TOOLCHAIN_PATH environment variable. See README.md for download instructions.")
endif()

set(TOOLCHAIN_PREFIX ${CH32_TOOLCHAIN_PATH})

# Forward this variable into CMake's internal try_compile scratch builds
# (e.g. compiler ABI detection) — otherwise those sub-invocations don't see
# the cache/env value and fail even though the real compiler is fine.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES CH32_TOOLCHAIN_PATH)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}/bin/riscv-none-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}/bin/riscv-none-elf-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}/bin/riscv-none-elf-gcc)
set(CMAKE_AR ${TOOLCHAIN_PREFIX}/bin/riscv-none-elf-ar)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}/bin/riscv-none-elf-objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}/bin/riscv-none-elf-objdump)
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}/bin/riscv-none-elf-size)

# Prevent CMake from testing the toolchain
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
