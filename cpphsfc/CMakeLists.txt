cmake_minimum_required(VERSION 2.8)
project(cpphsfc)

#---------------------------------------------------
# Building
#---------------------------------------------------

find_package(Boost 1.4 REQUIRED COMPONENTS unit_test_framework)
include_directories("${Boost_INCLUDE_DIRS}")

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${libhsfc_SOURCE_DIR}/src")

add_library(cpphsfc src/hsfc.cpp $<TARGET_OBJECTS:hsfcobj>)
target_link_libraries(cpphsfc ${Boost_LIBRARIES})

#---------------------------------------------------
# Installing
#---------------------------------------------------

FILE(GLOB incfiles "${CMAKE_CURRENT_SOURCE_DIR}/hsfc/*.h")
INSTALL(FILES ${incfiles} DESTINATION include/hsfc)
INSTALL(TARGETS cpphsfc ARCHIVE DESTINATION lib)

#---------------------------------------------------
# Testing
#---------------------------------------------------

enable_testing()

add_executable(cpphsfc-test test/cpphsfc-test.cpp)
target_link_libraries(cpphsfc-test cpphsfc ${Boost_LIBRARIES})

add_test(
  NAME HSFCTest 
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/test"
  COMMAND cpphsfc-test)
