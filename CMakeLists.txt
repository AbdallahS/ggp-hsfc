cmake_minimum_required(VERSION 2.8)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "DEBUG")
  #set(CMAKE_BUILD_TYPE "RELEASE")
  #set(CMAKE_BUILD_TYPE "RELWITHDEBINFO")
  #set(CMAKE_BUILD_TYPE "MINSIZEREL")
endif()

project(hsfc)

add_subdirectory(libhsfc)
add_subdirectory(cpphsfc)
add_subdirectory(pyhsfc)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

enable_testing()