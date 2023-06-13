cmake_minimum_required(VERSION 3.13)

#set path for the pico sdk
set(PICO_SDK_PATH /home/ankit/pi_pico/pico-sdk/)

# initialize the SDK directly
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

#set path for freeRtos kernel
set(FREERTOS_KERNEL_PATH /home/ankit/pi_pico/FreeRTOS-SMP-Demos/FreeRTOS/Source)

#setting board type to pico w
set(PICO_BOARD pico_w)



include(FreeRTOS_Kernel_import.cmake)

project(blinky C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
# Add main
add_subdirectory(blinky)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )
