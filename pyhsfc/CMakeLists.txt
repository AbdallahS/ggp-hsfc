cmake_minimum_required(VERSION 2.8)
project(pyhsfc)

#----------------------------------------------------
# Building python HSFC
#----------------------------------------------------

find_package(Boost 1.45 COMPONENTS python)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Unable to find correct Boost version. Did you set BOOST_ROOT?")
endif()

set(PYTHON_BASE_VERSION 2.7)
find_package(PythonLibs ${PYTHON_BASE_VERSION} REQUIRED)
if (NOT PYTHONLIBS_FOUND)
  message(FATAL_ERROR "Unable to find correct Python 2.7")
endif()

#execute_process(COMMAND python -V ERROR_VARIABLE PYTHON_VERSION)
#string(REGEX REPLACE "^Python +([^\\.]+\\.[^\\.]+).*$" "\\1" PYTHON_BASE_VERSION "${PYTHON_VERSION}")
#include_directories("${Boost_INCLUDE_DIRS}" "/usr/include/python${PYTHON_BASE_VERSION}")

include_directories("${Boost_INCLUDE_DIRS}" "${PYTHON_INCLUDE_PATH}")
include_directories("${cpphsfc_SOURCE_DIR}" "${libhsfc_SOURCE_DIR}/src")
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

add_library(pyhsfc SHARED src/pyhsfc.cpp $<TARGET_OBJECTS:cpphsfc_shobj> $<TARGET_OBJECTS:hsfc_shobj>)
set_target_properties(pyhsfc PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(pyhsfc PROPERTIES PREFIX "")
target_link_libraries(pyhsfc ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})

#----------------------------------------------------
# Installing python HSFC
#----------------------------------------------------

install(TARGETS pyhsfc LIBRARY DESTINATION "lib/python${PYTHON_BASE_VERSION}/site-packages")


