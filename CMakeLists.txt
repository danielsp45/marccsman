cmake_minimum_required(VERSION 3.10)
project(marccsman)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for headers.
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/adapters)

# Gather core source files.
file(GLOB CORE_SRC "${CMAKE_SOURCE_DIR}/src/*.cc")

# Create a static library for the core code.
add_library(marccsman_core STATIC ${CORE_SRC})

# Create the main executable from main.cc.
add_executable(marccsman ${CMAKE_SOURCE_DIR}/src/main.cc)

# Export all symbols so that the plugin can resolve KVStoreFactory symbols.
target_link_options(marccsman PRIVATE -rdynamic)

# Link the core library and pthread.
target_link_libraries(marccsman PRIVATE marccsman_core pthread)

# Optionally, if you need to link any plugins that are statically built:
# target_link_libraries(marccsman PRIVATE some_static_plugin)

# Define adapter directories.
set(ADAPTER_DIRS rocksdb dummy leveldb)

# Add subdirectories for each adapter.
foreach(dir ${ADAPTER_DIRS})
    if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/adapters/${dir}")
        add_subdirectory(adapters/${dir})
    endif()
endforeach()
