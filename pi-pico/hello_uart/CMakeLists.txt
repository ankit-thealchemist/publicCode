cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH /home/ankit/pi_pico/pico-sdk/)
# initialize the SDK directly
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(hello_uart C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
# Add main
add_subdirectory(main)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )
