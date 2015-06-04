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
  src/kpWFT.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/src)

# This is a drastic way to remove the large amount of (uncontrolled)
# debugging output.
add_definitions( -DREMOVE_PRINTF_MADNESS )


add_library(hsfc_shobj OBJECT ${LIBHSFC_SRC})
add_library(hsfc_stobj OBJECT ${LIBHSFC_SRC})
set_target_properties(hsfc_shobj PROPERTIES COMPILE_FLAGS "-fPIC")


#add_library(hsfc_shared SHARED $<TARGET_OBJECTS:hsfc_shobj>)
#add_library(hsfc_static $<TARGET_OBJECTS:hsfc_stobj>)
#set_target_properties(hsfc_shared hsfc_static PROPERTIES OUTPUT_NAME hsfc)

#---------------------------------------------------
# Fix the header files for later installation
#---------------------------------------------------

set(LIBHSFC_HEADERS
  hsfcAPI.h
  hsfcEngine.h
  hsfcGDL.h
  hsfcGrinder.h
  hsfcLexicon.h
  hsfcSchema.h
  hsfcState.h
  kpWFT.h
)  

set(HDR_ORIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIBHSFC_PATCHED_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/patched_headers")
set(HDR_DEST_DIR "${LIBHSFC_PATCHED_HEADERS_DIR}/hsfc/impl/")
set(PATCH_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/patch_hsfc_headers.py")
file(MAKE_DIRECTORY "${HDR_DEST_DIR}")
foreach(hdrfile ${LIBHSFC_HEADERS})
   list(APPEND LIBHSFC_PATCHED_HEADERS "${HDR_DEST_DIR}/${hdrfile}")
   add_custom_command(
     OUTPUT "${HDR_DEST_DIR}/${hdrfile}"
     COMMAND python "${PATCH_SCRIPT}" "${HDR_ORIG_DIR}/${hdrfile}" "${HDR_DEST_DIR}/${hdrfile}"
     DEPENDS "${HDR_ORIG_DIR}/${hdrfile}"
     )
endforeach(hdrfile)
add_custom_target(patched_libhsfc_headers ALL DEPENDS ${LIBHSFC_PATCHED_HEADERS})

#---------------------------------------------------
# Installation
#---------------------------------------------------

#FILE(GLOB incfiles "${HDR_DEST_DIR}/*.h")
INSTALL(FILES ${LIBHSFC_PATCHED_HEADERS} DESTINATION include/hsfc/impl)
#INSTALL(TARGETS hsfc_shared LIBRARY DESTINATION lib)
#INSTALL(TARGETS hsfc_static ARCHIVE DESTINATION lib)

#---------------------------------------------------
# Testing
#---------------------------------------------------

#add_executable(hsfc-example src/Example.cpp)
#target_link_libraries(hsfc-example hsfc)

