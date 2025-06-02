# Add image data

add_library(image_data STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/images/images.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/fonts/fonts.cpp
  )

set_target_properties(image_data PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 20)

target_include_directories(image_data PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/../src
)

target_include_directories(image_data PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}/includes/
)

target_link_libraries(image_data
  hardware_dma
  pico_stdlib
)

# Run image_gs4 generator
file(GLOB IMAGES ${CMAKE_CURRENT_LIST_DIR}/**/*.png)
file(GLOB JSON_FONTS ${CMAKE_CURRENT_LIST_DIR}/**/*.json)
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/images/images.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/images/images.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/fonts/fonts.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/fonts/fonts.cpp
  COMMAND
    python ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/../tools/image_gs4
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets
  DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/../tools/image_gs4/__main__.py
    ${IMAGES}
    ${JSON_FONTS}
  COMMENT "Running image_gs4"
)

add_custom_target(
  image_gs4 ALL
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/images/images.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/images/images.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/fonts/fonts.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/fonts/fonts.cpp
)

add_dependencies(image_data image_gs4)
