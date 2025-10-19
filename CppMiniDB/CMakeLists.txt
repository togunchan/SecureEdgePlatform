cmake_minimum_required(VERSION 3.10)
project(CppMiniDB)

set(CMAKE_CXX_STANDARD 20)

# Source files
add_library(minidb
    src/MiniDB.cpp
)

# Add nlohmann/json include path (single header mode)
target_include_directories(minidb
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/../third_party/json/single_include
)

# Test executable
add_executable(test_minidb tests/test_minidb.cpp)

# Include test & third-party headers
target_include_directories(test_minidb 
    PRIVATE 
    tests
    ${PROJECT_SOURCE_DIR}/../third_party/Catch2/src
    ${PROJECT_SOURCE_DIR}/CppMiniDB/include
    ${PROJECT_SOURCE_DIR}/../third_party/json/single_include
)

# Link the library to the test
target_link_libraries(test_minidb 
    PRIVATE 
    minidb 
    Catch2::Catch2WithMain)

# Enable Catch2 integration
include(CTest)
include(Catch)
catch_discover_tests(test_minidb)
