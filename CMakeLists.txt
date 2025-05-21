cmake_minimum_required(VERSION 4.0)

SET(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
set(PICO_CXX_ENABLE_EXCEPTIONS 0)


include(pico-sdk/pico_sdk_init.cmake)

project(pico_game LANGUAGES C CXX ASM)


add_executable(pico_game
        src/main.cpp
)

set_target_properties(pico_game PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17)

target_link_libraries(pico_game pico_stdlib)

set(EXPORT_COMPILE_COMMANDS true)

pico_sdk_init()
pico_add_extra_outputs(pico_game)