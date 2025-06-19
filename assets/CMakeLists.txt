# Add image data

add_library(image_data STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/images.cpp
  # ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/fonts.cpp
  )

set_target_properties(image_data PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 23)

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

# Run image_importer
file(GLOB IMAGES ${CMAKE_CURRENT_LIST_DIR}/**/*.png)
file(GLOB JSON_FONTS ${CMAKE_CURRENT_LIST_DIR}/**/*.json)
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/images.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/images.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/fonts.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/fonts.cpp
  COMMAND
    python ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/../tools/image_importer
    -i ${CMAKE_CURRENT_LIST_DIR}/images
    -o ${CMAKE_CURRENT_BINARY_DIR}/includes/assets
    -f GS4A1
  DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/../tools/image_importer/__main__.py
    ${IMAGES}
    ${JSON_FONTS}
  COMMENT "Running image_importer"
)

add_custom_target(
  import_images ALL
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/images.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/images.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/fonts.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/includes/assets/fonts.cpp
)

add_dependencies(image_data import_images)
