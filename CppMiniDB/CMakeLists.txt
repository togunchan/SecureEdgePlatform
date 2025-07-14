cmake_minimum_required(VERSION 3.10)
project(CppMiniDB)

set(CMAKE_CXX_STANDARD 17)

# Include headers
include_directories(include)

# Source files
add_library(minidb
    src/MiniDB.cpp
)

# Test executable
add_executable(test_minidb
    tests/test_minidb.cpp
)

# Link the library to the test
target_link_libraries(test_minidb minidb)