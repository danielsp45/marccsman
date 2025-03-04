cmake_minimum_required(VERSION 3.10)
project(rocksdb_adapter)

# Build the RocksDB adapter as a shared library.
add_library(rocksdb_adapter SHARED rocksdb_adapter.cc)

# Include core headers if needed.
target_include_directories(rocksdb_adapter PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link against the core library so that KVStoreFactory and other common code are available.
target_link_libraries(rocksdb_adapter PRIVATE marccsman_core)

# Optionally, link against any additional libraries (e.g., RocksDB) here.
# target_link_libraries(rocksdb_adapter PRIVATE rocksdb)
