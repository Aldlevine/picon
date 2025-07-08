cmake_minimum_required(VERSION 4.0)

# Main executable {{{

find_package(SDL3 REQUIRED)

add_executable(${PROJECT_NAME}
        src/main.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 23)

target_link_libraries(${PROJECT_NAME} PRIVATE
    image_data
    SDL3::SDL3)

# }}} Main Executable


