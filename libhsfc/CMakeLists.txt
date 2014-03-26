cmake_minimum_required(VERSION 2.8)
project(libhsfc)

#---------------------------------------------------
# Building
#---------------------------------------------------

set(LIBHSFC_SRC 
  src/hsfcAPI.cpp
  src/hsfcEngine.cpp  
  src/hsfcGDL.cpp  
  src/hsfcGrinder.cpp  
  src/hsfcLexicon.cpp  
  src/hsfcSchema.cpp  
  src/hsfcState.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/src)

add_library(hsfc_shobj OBJECT ${LIBHSFC_SRC})
add_library(hsfc_stobj OBJECT ${LIBHSFC_SRC})
set_target_properties(hsfc_shobj PROPERTIES COMPILE_FLAGS "-fPIC")

#add_library(hsfc_shared SHARED $<TARGET_OBJECTS:hsfc_shobj>)
#add_library(hsfc_static $<TARGET_OBJECTS:hsfc_stobj>)
#set_target_properties(hsfc_shared hsfc_static PROPERTIES OUTPUT_NAME hsfc)

#---------------------------------------------------
# Installing
#---------------------------------------------------

FILE(GLOB incfiles "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
INSTALL(FILES ${incfiles} DESTINATION include)
#INSTALL(TARGETS hsfc_shared LIBRARY DESTINATION lib)
#INSTALL(TARGETS hsfc_static ARCHIVE DESTINATION lib)

#---------------------------------------------------
# Testing
#---------------------------------------------------

#add_executable(hsfc-example src/Example.cpp)
#target_link_libraries(hsfc-example hsfc)

