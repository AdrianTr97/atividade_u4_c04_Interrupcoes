


cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
project(atividadeu4c04 C CXX ASM)
pico_sdk_init()
add_executable(atividadeu4c04 atividadeu4c04.c )
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(atividadeu4c04 ${CMAKE_CURRENT_LIST_DIR}/atividadeu4c04.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
target_sources(atividadeu4c04 PRIVATE atividadeu4c04.c)
target_link_libraries(atividadeu4c04 PRIVATE pico_stdlib hardware_pio)
pico_add_extra_outputs(atividadeu4c04) 
