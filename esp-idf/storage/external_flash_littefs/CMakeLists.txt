# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# external SPI flash driver not currently supported for ESP32-S2
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS D:/IOTCode/libraries/extra_components/esp_littlefs)
set (CMAKE_CXX_STANDARD 17)
project(external_flash_littleFs)
