cmake_minimum_required(VERSION 3.10)
project(marccsman)

# Set the path to the RocksDB CMake configuration files installed by Nix.
# Adjust the path if your RocksDB version or location differs.
set(RocksDB_DIR "/nix/store/9q13xc9zf421k8401aslign8549wycv8-rocksdb-9.7.4/lib/cmake/RocksDB")

# Find RocksDB using the config file.
find_package(RocksDB REQUIRED)

# Create the marccsman_core library (your core benchmark code)
add_library(marccsman_core SHARED 
    src/some_core_file.cc
    # Add other source files here...
)

# Include our source directory and RocksDB's include directories.
target_include_directories(marccsman_core PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${RocksDB_INCLUDE_DIRS}
)

# Link marccsman_core against RocksDB.
target_link_libraries(marccsman_core PRIVATE ${RocksDB_LIBRARIES})
