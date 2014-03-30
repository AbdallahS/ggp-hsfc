cmake_minimum_required(VERSION 2.8)
project(examples)
set(CMAKE_BUILD_TYPE "Debug")

find_package(Boost 1.4 REQUIRED)
include_directories("${Boost_INCLUDE_DIRS}")

#include_directories("${PROJECT_SOURCE_DIR}")
#include_directories("${PROJECT_SOURCE_DIR}/../include")

add_executable(example1 example1.cpp)
target_link_libraries(example1 ${Boost_LIBRARIES} -lcpphsfc)
