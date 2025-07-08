cmake_minimum_required(VERSION 4.0)

set(PICO_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk")
set(PICO_CXX_ENABLE_EXCEPTIONS 0)
include(pico-sdk/pico_sdk_init.cmake)

pico_sdk_init()

# Main executable {{{

add_executable(${PROJECT_NAME}
        src/main.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 23)

# target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)

target_link_libraries(${PROJECT_NAME} PRIVATE
        hardware_dma
        hardware_flash
        hardware_interp
        hardware_pio
        hardware_spi
        pico_rand
        pico_stdlib
        image_data)

pico_set_program_name(${PROJECT_NAME} "${PROJECT_NAME}")
pico_set_program_version(${PROJECT_NAME} "0.1")
pico_add_extra_outputs(${PROJECT_NAME})

# }}} Main Executable
