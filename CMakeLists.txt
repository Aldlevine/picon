cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_SCAN_FOR_MODULES false)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

# detect native platform
if(PICON_PLATFORM_NATIVE)
        if(LINUX)
                set(PICON_PLATFORM_LINUX true)
        elseif(WIN32)
                set(PICON_PLATFORM_WIN32 true)
        endif()
endif()

# platform preludes
if(PICON_PLATFORM_PICO)
        include(pico-sdk/pico_sdk_init.cmake)
endif()

project(picon LANGUAGES C CXX ASM)

# assets
add_subdirectory(assets)

# platform targets
if(PICON_PLATFORM_PICO)
        set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
        set(PICO_CXX_ENABLE_EXCEPTIONS 0)
        include(cmake/pico.cmake)
        target_compile_definitions(${PROJECT_NAME} PUBLIC PICON_PLATFORM_PICO)

elseif(PICON_PLATFORM_LINUX)
        include(cmake/linux.cmake)
        target_compile_definitions(${PROJECT_NAME} PUBLIC PICON_PLATFORM_LINUX)

endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 23)

# platform compilation db
file(CREATE_LINK
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
        SYMBOLIC
        )