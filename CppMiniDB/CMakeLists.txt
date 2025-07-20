cmake_minimum_required(VERSION 3.10)
project(CppMiniDB)

set(CMAKE_CXX_STANDARD 17)

# Include headers
include_directories(include)

# Source files
add_library(minidb
    src/MiniDB.cpp
)

add_subdirectory(../third_party/Catch2 Catch2-build)

# Test executable
add_executable(test_minidb tests/test_minidb.cpp)

# Include catch2 header path
target_include_directories(test_minidb 
    PRIVATE 
    tests
    ${PROJECT_SOURCE_DIR}/third_party/Catch2/src
    ${PROJECT_SOURCE_DIR}/CppMiniDB/include
)

# Link the library to the test
target_link_libraries(test_minidb 
    PRIVATE 
    minidb 
    Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(test_minidb)