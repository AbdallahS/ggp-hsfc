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

add_library(hsfcobj OBJECT ${LIBHSFC_SRC})
add_library(hsfc $<TARGET_OBJECTS:hsfcobj>)

#---------------------------------------------------
# Installing
#---------------------------------------------------

FILE(GLOB incfiles "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
INSTALL(FILES ${incfiles} DESTINATION include)
INSTALL(TARGETS hsfc ARCHIVE DESTINATION lib)

#---------------------------------------------------
# Testing
#---------------------------------------------------

#add_executable(hsfc-example src/Example.cpp)
#target_link_libraries(hsfc-example hsfc)

