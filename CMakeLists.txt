cmake_minimum_required(VERSION 2.8)
if(NOT CMAKE_BUILD_TYPE)
  #set(CMAKE_BUILD_TYPE "Debug")
  set(CMAKE_BUILD_TYPE "Release")
  #set(CMAKE_BUILD_TYPE "RelWithDebInfo")
  #set(CMAKE_BUILD_TYPE "MinSizeRel")
endif()

project(hsfc)

add_subdirectory(libhsfc)
add_subdirectory(cpphsfc)
add_subdirectory(pyhsfc)

# Only set the CMAKE_INSTALL_PREFIX if the user hasn't
# specified a location
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
endif()

enable_testing()
