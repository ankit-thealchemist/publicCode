 
cmake_minimum_required (VERSION 3.2)
project (test-project)

add_executable (test-project main/main.c)
target_link_libraries(test-project PUBLIC cjson)
