cmake_minimum_required(VERSION 3.10)
project(marccsman)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for headers.
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/plugins)

# Gather core source files.
file(GLOB CORE_SRC "${CMAKE_SOURCE_DIR}/src/*.cc")

# Create a static library for the core code.
add_library(marccsman_core STATIC ${CORE_SRC})

# Create the main executable from main.cc.
add_executable(marccsman ${CMAKE_SOURCE_DIR}/src/main.cc)

# Link the core library, pthread, and plugin libraries into the executable.
# Adjust leveldb_adapter if you have it; otherwise, link only rocksdb_adapter.
target_link_libraries(marccsman PRIVATE marccsman_core pthread rocksdb_adapter)

# Define plugin directories.
set(PLUGIN_DIRS rocksdb leveldb)

# Add subdirectories for each plugin.
foreach(dir ${PLUGIN_DIRS})
    if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins/${dir}")
        add_subdirectory(plugins/${dir})
    endif()
endforeach()
