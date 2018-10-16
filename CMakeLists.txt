cmake_minimum_required(VERSION 3.0)
project(buzzlight)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories("include")

set(BUZZ_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(BUZZ_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUZZ_BINARY_DIR})

add_subdirectory(tool)
