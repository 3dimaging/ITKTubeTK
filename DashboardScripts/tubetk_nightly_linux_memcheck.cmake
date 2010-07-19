cmake_minimum_required(VERSION 2.6)

set( CTEST_SITE "${SITE_NAME}" )
set( CTEST_BUILD_NAME "${SITE_BUILD_NAME}" )
set( CTEST_BUILD_CONFIGURATION "${SITE_BUILD_TYPE}" )

set( CTEST_TEST_TIMEOUT 1500 )

set( CTEST_CMAKE_GENERATOR "Unix Makefiles" )
set( CTEST_NOTES_FILES "${SITE_SCRIPT_DIR}/${CTEST_SCRIPT_NAME}" )
set( CTEST_SOURCE_DIRECTORY "${SITE_SOURCE_DIR}" )
set( CTEST_BINARY_DIRECTORY "${SITE_BINARY_DIR}" )

set( ENV{DISPLAY} ":0" )

set( CTEST_BUILD_COMMAND "${SITE_MAKE_COMMAND}" )
set( CTEST_CMAKE_COMMAND "${SITE_CMAKE_COMMAND}" )
set( CTEST_QMAKE_COMMAND "${SITE_QMAKE_COMMAND}" )
set( CTEST_UPDATE_COMMAND "${SITE_UPDATE_COMMAND}" )

#
# Memcheck
#
set( CTEST_MEMORYCHECK_COMMAND "${SITE_MEMORYCHECK_COMMAND}" )
set( CTEST_MEMORYCHECK_SUPRESSIONS_FILE 
       "${SITE_SCRIPT_DIR}/tubetk_valgrind_suppression.txt" )
set( CTEST_MEMORYCHECK_COMMAND_OPTIONS "--gen-suppressions=all --trace-children=yes -q --leak-check=yes --show-reachable=yes --num-callers=50" ) 
set( MEMORYCHECK_OPTIONS "-g -O0 -ggdb" )

#
# Coverage
#
set( CTEST_COVERAGE_COMMAND "${SITE_COVERAGE_COMMAND}" )
set( COVERAGE_OPTIONS "-fprofile-arcs -ftest-coverage -lgcov" )

ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})

file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
  BUILD_TESTING:BOOL=ON
  BUILD_SHARED_LIBS:BOOL=ON
  CMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}
  SITE:STRING=${CTEST_SITE}
  BUILDNAME:STRING=${CTEST_BUILD_NAME}
  MAKECOMMAND:STRING=${CTEST_BUILD_COMMAND}
  QT_QMAKE_EXECUTABLE:FILEPATH=${CTEST_QMAKE_COMMAND}
  CMAKE_CXX_FLAGS:STRING=-fPIC -fdiagnostics-show-option -W -Wall -Wextra -Wshadow -Wno-system-headers -Wwrite-strings -Wno-deprecated -Woverloaded-virtual ${MEMORYCHECK_OPTIONS} ${COVERAGE_OPTIONS} )
  CMAKE_C_FLAGS:STRING=-fPIC -fdiagnostics-show-option -W -Wall -Wextra -Wshadow -Wno-system-headers -Wwrite-strings ${MEMORYCHECK_OPTIONS} ${COVERAGE_OPTIONS} )
  ")

ctest_start(Nightly)
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}")
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}")
ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}")
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}/TubeTK-Build")
ctest_submit()
