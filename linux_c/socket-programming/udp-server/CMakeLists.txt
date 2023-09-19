 
cmake_minimum_required (VERSION 3.2)

# Get the current source directory
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# Set the project name to the folder name
project(${PROJECT_NAME} C)

add_executable (${PROJECT_NAME} main/main.c)