cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_SCAN_FOR_MODULES false)
set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
set(PICO_CXX_ENABLE_EXCEPTIONS 0)
set(EXPORT_COMPILE_COMMANDS true)

include(pico-sdk/pico_sdk_init.cmake)

project(pico_game LANGUAGES C CXX ASM)

add_subdirectory(assets)

pico_sdk_init()

# Main executable {{{

add_executable(${PROJECT_NAME}
        src/main.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(pico_game PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 20)

target_link_libraries(pico_game PRIVATE
        hardware_dma
        hardware_flash
        hardware_pio
        hardware_spi
        pico_stdlib
        image_data)

pico_set_program_name(${PROJECT_NAME} "${PROJECT_NAME}")
pico_set_program_version(${PROJECT_NAME} "0.1")

pico_add_extra_outputs(pico_game)

# }}} Main Executable
