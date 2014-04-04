cmake_minimum_required(VERSION 2.8)
project(cpphsfc)

#---------------------------------------------------
# Building
#---------------------------------------------------

find_package(Boost 1.4 COMPONENTS serialization filesystem system REQUIRED )
include_directories("${Boost_INCLUDE_DIRS}")

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${libhsfc_BINARY_DIR}/patched_headers")

set(CPPHSFC_SRC
  src/hsfc.cpp
  src/hsfcwrapper.cpp
  src/sexprtoflat.cpp
)

#add_library(cpphsfc src/hsfc.cpp $<TARGET_OBJECTS:hsfcobj>)
add_library(cpphsfc_stobj OBJECT ${CPPHSFC_SRC})
add_library(cpphsfc_shobj OBJECT ${CPPHSFC_SRC})
set_target_properties(cpphsfc_shobj PROPERTIES COMPILE_FLAGS "-fPIC")

# Add the generated libhsfc header files as a dependency
add_dependencies(cpphsfc_stobj patched_libhsfc_headers)
add_dependencies(cpphsfc_shobj patched_libhsfc_headers)

#add_library(cpphsfc_shared SHARED $<TARGET_OBJECTS:cpphsfc_shobj> $<TARGET_OBJECTS:hsfc_shobj>)
add_library(cpphsfc_static $<TARGET_OBJECTS:cpphsfc_stobj> $<TARGET_OBJECTS:hsfc_stobj>)
#set_target_properties(cpphsfc_shared cpphsfc_static PROPERTIES OUTPUT_NAME cpphsfc)
set_target_properties(cpphsfc_static PROPERTIES OUTPUT_NAME cpphsfc)

#target_link_libraries(cpphsfc_shared ${Boost_LIBRARIES})
target_link_libraries(cpphsfc_static ${Boost_LIBRARIES})

#---------------------------------------------------
# Installing
#---------------------------------------------------

file(GLOB incfiles1 "${CMAKE_CURRENT_SOURCE_DIR}/hsfc/*.h")
install(FILES ${incfiles1} DESTINATION include/hsfc)
file(GLOB incfiles2 "${CMAKE_CURRENT_SOURCE_DIR}/hsfc/impl/*.h")
install(FILES ${incfiles2} DESTINATION include/hsfc/impl)

install(TARGETS cpphsfc_static ARCHIVE DESTINATION lib)
#install(TARGETS cpphsfc_shared LIBRARY DESTINATION lib)

#---------------------------------------------------
# Testing
#---------------------------------------------------

enable_testing()
add_subdirectory(test)


#add_executable(cpphsfc-test test/cpphsfc-test.cpp)
#target_link_libraries(cpphsfc-test cpphsfc_static ${Boost_LIBRARIES})

#add_test(
#  NAME CppHSFCTest 
#  WORKING_DIRECTORY "${hsfc_SOURCE_DIR}/testing"
#  COMMAND cpphsfc-test "--log_level=test_suite"
#)
