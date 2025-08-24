cmake_minimum_required(VERSION 3.10)
project(CppMiniDB)

set(CMAKE_CXX_STANDARD 17)

# public headers
add_library(sensors INTERFACE)
target_include_directories(sensors INTERFACE)
    target_include_directories(sensors INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

