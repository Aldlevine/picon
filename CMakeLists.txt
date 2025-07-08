cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_SCAN_FOR_MODULES false)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

if(PICON_PLATFORM_NATIVE)
        if(LINUX)
                set(PICON_PLATFORM_LINUX true)
        elseif(MSVC)
                set(PICON_PLATFORM_WINDOWS true)
        endif()
endif()

if(PICON_PLATFORM_PICO)
        include(pico-sdk/pico_sdk_init.cmake)
endif()

project(picon LANGUAGES C CXX ASM)


# set(CMAKE_C_COMPILER gcc)
# set(CMAKE_CXX_COMPILER g++)

add_subdirectory(assets)

if(PICON_PLATFORM_PICO)
        set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
        set(PICO_CXX_ENABLE_EXCEPTIONS 0)

        include(cmake/pico.cmake)

        target_compile_definitions(${PROJECT_NAME} PUBLIC PICON_PLATFORM_PICO)

elseif(PICON_PLATFORM_LINUX)
        include(cmake/linux.cmake)
        target_compile_definitions(${PROJECT_NAME} PUBLIC PICON_PLATFORM_LINUX)

endif()

file(CREATE_LINK
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
        SYMBOLIC
        )