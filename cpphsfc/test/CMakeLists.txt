cmake_minimum_required(VERSION 2.8)

#---------------------------------------------------
# Testing
#---------------------------------------------------

find_package(Boost 1.4 REQUIRED COMPONENTS system serialization filesystem unit_test_framework)
include_directories("${Boost_INCLUDE_DIRS}")

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${libhsfc_SOURCE_DIR}/src")

enable_testing()

add_executable(sexprtoflat-test sexprtoflat-test.cpp)
target_link_libraries(sexprtoflat-test cpphsfc_static ${Boost_LIBRARIES})

add_executable(cpphsfc-test cpphsfc-test.cpp)
target_link_libraries(cpphsfc-test cpphsfc_static ${Boost_LIBRARIES})

  add_executable(portable-test portable-test.cpp)
  target_link_libraries(portable-test cpphsfc_static ${Boost_LIBRARIES})

  add_executable(nineboardtictactoe-test nineboardtictactoe-test.cpp)
  target_link_libraries(nineboardtictactoe-test cpphsfc_static ${Boost_LIBRARIES})

add_executable(playermoves-test playermoves-test.cpp)
target_link_libraries(playermoves-test cpphsfc_static ${Boost_LIBRARIES})

# Search for gadelac
execute_process(COMMAND which gadelac OUTPUT_VARIABLE GADELAC_PATH)
if (GADELAC_PATH MATCHES "^\\s*$")
  message("Gadelac does not appear to be present. Will skip the tests of gadelac integration")
else ()
  message("Found Gadelac in the PATH. Enabling Gadelac integration tests")

  add_executable(gadelac-integration-test gadelac-integration-test.cpp)
  target_link_libraries(gadelac-integration-test cpphsfc_static ${Boost_LIBRARIES})
endif ()

add_executable(statistical-test statistical-test.cpp)
target_link_libraries(statistical-test cpphsfc_static ${Boost_LIBRARIES})

add_test(
  NAME CppHSFCTest
  COMMAND cpphsfc-test "--log_level=test_suite"
)

add_test(
  NAME CppHSFCPortable
  COMMAND portable-test "--log_level=test_suite"
)

add_test(
  NAME CppHSFCNineBoardTicTacToe
  COMMAND nineboardtictactoe-test "--log_level=test_suite"
)

add_test(
  NAME CppHSFCPlayerMoves
  COMMAND playermoves-test "--log_level=test_suite"
)

add_test(
  NAME TermFormatConversionTest
  COMMAND sexprtoflat-test "--log_level=test_suite"
)

if (NOT GADELAC_PATH MATCHES "^\\s*$")
  add_test(
    NAME GadelacIntegrationTest
    COMMAND gadelac-integration-test "--log_level=test_suite"
  )

  add_test(
    NAME StatisticalTest
    COMMAND statistical-test "--log_level=test_suite"
  )
endif (NOT GADELAC_PATH MATCHES "^\\s*$")
