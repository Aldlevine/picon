cmake_minimum_required(VERSION 4.0)


set(CMAKE_CXX_SCAN_FOR_MODULES false)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

if(PICON_PLATFORM_PICO)
        set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
        set(PICO_CXX_ENABLE_EXCEPTIONS 0)
        include(pico-sdk/pico_sdk_init.cmake)
endif()

project(pico_game LANGUAGES C CXX ASM)

add_subdirectory(assets)

if(PICON_PLATFORM_PICO)
        include(cmake/pico.cmake)
endif()