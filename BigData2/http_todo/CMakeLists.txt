cmake_minimum_required(VERSION 3.10)
project(http_todo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -Wall -Wextra")

add_subdirectory(lib/cJSON)

add_executable(http-todo main.c)

target_link_libraries(http-todo PRIVATE cjson)

target_include_directories(http-todo PRIVATE lib/cJSON)
