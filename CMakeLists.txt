cmake_minimum_required(VERSION 2.8)
project(hsfc)

add_subdirectory(libhsfc)
add_subdirectory(cpphsfc)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

enable_testing()