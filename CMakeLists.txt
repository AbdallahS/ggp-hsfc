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

#set(CMAKE_CXX_FLAGS "-std=c++11")

add_subdirectory(libhsfc)           # Build the base libhsfc
add_subdirectory(cpphsfc)           # Build the cleaner C++ wrapper


# Test for python
set(PYTHON_VERSION 2.7)
find_package(PythonLibs ${PYTHON_VERSION})
if (NOT PYTHONLIBS_FOUND)
  message(WARNING "Unable to find Python ${PYTHON_VERSION}")
  set(DISABLE_PYHSFC 1)
endif()

# If no python or DISABLE_PYHSFC is set then don't build pyhsfc
if (DISABLE_PYHSFC)
  message(STATUS "Not building PyHSFC")
else()
  add_subdirectory(pyhsfc)            # Build the Python wrapper
endif()

# Only set the CMAKE_INSTALL_PREFIX if the user hasn't
# specified a location
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
endif()

enable_testing()
