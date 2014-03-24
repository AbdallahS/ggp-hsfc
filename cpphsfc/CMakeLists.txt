cmake_minimum_required(VERSION 2.8)
project(cpphsfc)

#---------------------------------------------------
# Building
#---------------------------------------------------

find_package(Boost 1.4 REQUIRED COMPONENTS unit_test_framework)
include_directories("${Boost_INCLUDE_DIRS}")

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${libhsfc_SOURCE_DIR}/src")

#add_library(cpphsfc src/hsfc.cpp $<TARGET_OBJECTS:hsfcobj>)
add_library(cpphsfc_stobj OBJECT src/hsfc.cpp)
add_library(cpphsfc_shobj OBJECT src/hsfc.cpp)
set_target_properties(cpphsfc_shobj PROPERTIES COMPILE_FLAGS "-fPIC")

add_library(cpphsfc_shared SHARED $<TARGET_OBJECTS:cpphsfc_shobj> $<TARGET_OBJECTS:hsfc_shobj>)
add_library(cpphsfc_static $<TARGET_OBJECTS:cpphsfc_stobj> $<TARGET_OBJECTS:hsfc_stobj>)
set_target_properties(cpphsfc_shared cpphsfc_static PROPERTIES OUTPUT_NAME cpphsfc)

target_link_libraries(cpphsfc_shared ${Boost_LIBRARIES})
target_link_libraries(cpphsfc_static ${Boost_LIBRARIES})

#---------------------------------------------------
# Installing
#---------------------------------------------------

file(GLOB incfiles "${CMAKE_CURRENT_SOURCE_DIR}/hsfc/*.h")
install(FILES ${incfiles} DESTINATION include/hsfc)
install(TARGETS cpphsfc_static ARCHIVE DESTINATION lib)
install(TARGETS cpphsfc_shared LIBRARY DESTINATION lib)

#---------------------------------------------------
# Testing
#---------------------------------------------------

enable_testing()

add_executable(cpphsfc-test test/cpphsfc-test.cpp)
target_link_libraries(cpphsfc-test cpphsfc_static ${Boost_LIBRARIES})

add_test(
  NAME HSFCTest 
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/test"
  COMMAND cpphsfc-test)
