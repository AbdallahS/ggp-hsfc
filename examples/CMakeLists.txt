cmake_minimum_required(VERSION 2.8)
project(examples)
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


find_package(Boost 1.49 COMPONENTS system serialization filesystem REQUIRED)
include_directories("${Boost_INCLUDE_DIRS}")

#include_directories("${PROJECT_SOURCE_DIR}")
#include_directories("${PROJECT_SOURCE_DIR}/../include")

add_executable(example1 example1.cpp)
target_link_libraries(example1  -lcpphsfc ${Boost_LIBRARIES})

add_executable(playout playout.cpp)
target_link_libraries(playout  -lcpphsfc ${Boost_LIBRARIES})

add_executable(playout2 playout2.cpp)
target_link_libraries(playout2  -lcpphsfc ${Boost_LIBRARIES})

add_executable(play_to_termination play_to_termination.cpp)
target_link_libraries(play_to_termination  -lcpphsfc ${Boost_LIBRARIES})
