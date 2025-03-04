cmake_minimum_required(VERSION 3.10)
project(dummy_adapter)

add_library(dummy_adapter SHARED
    plugin.cc         # Registration function file.
    dummy_adapter.cc  # Dummy adapter implementation.
)

target_include_directories(dummy_adapter PRIVATE
    ${CMAKE_SOURCE_DIR}/adapters/dummy
    ${CMAKE_SOURCE_DIR}/src  # In case common headers are needed.
)

# Place the plugin in the build directory's adapters folder.
set_target_properties(dummy_adapter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/adapters"
)