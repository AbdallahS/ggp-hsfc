cmake_minimum_required(VERSION 2.8)
if(NOT CMAKE_BUILD_TYPE)
  #set(CMAKE_BUILD_TYPE "Debug")
  set(CMAKE_BUILD_TYPE "Release")
  #set(CMAKE_BUILD_TYPE "RelWithDebInfo")
  #set(CMAKE_BUILD_TYPE "MinSizeRel")
endif()

#-----------------------------------------------------------------
# Define flags for a Debug Profiler and a Release Profiler builds.
# -----------------------------------------------------------------
set(CMAKE_C_FLAGS_DPROFILER "${CMAKE_C_FLAGS_DEBUG} -pg")
set(CMAKE_CXX_FLAGS_DPROFILER "${CMAKE_CXX_FLAGS_DEBUG} -pg")
set(CMAKE_EXE_LINKER_FLAGS_DPROFILER "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg")
set(CMAKE_SHARED_LINKER_FLAGS_DPROFILER "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -pg")

set(CMAKE_C_FLAGS_RPROFILER "${CMAKE_C_FLAGS_RELEASE} -pg")
set(CMAKE_CXX_FLAGS_RPROFILER "${CMAKE_CXX_FLAGS_RELEASE} -pg")
set(CMAKE_EXE_LINKER_FLAGS_RPROFILER "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -pg")
set(CMAKE_SHARED_LINKER_FLAGS_RPROFILER "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -pg")

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
