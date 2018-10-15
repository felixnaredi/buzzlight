cmake_minimum_required(VERSION 3.0)
project(buzzlight)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories("include")

set(BUZZ_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

add_subdirectory(tool)
