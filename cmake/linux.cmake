cmake_minimum_required(VERSION 4.0)

find_package(SDL3 REQUIRED)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/config/convert_custom.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    image_data
    SDL3::SDL3)

