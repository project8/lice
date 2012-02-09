# CMakeLists for lice
# Author: N. Oblath

# minimum cmake version 2.6 required by the scripts to get Git information
cmake_minimum_required (VERSION 2.6)

# define the project
project (lice)

# find the curl library
find_package (CURL)
set (LIBS ${LIBS} ${CURL_LIBRARIES})

# check open_memstream exists; if not set the flag which will use MemstreamInterface
include (CheckFunctionExists)
check_function_exists(open_memstream HAVE_MEMSTREAM)
if (NOT HAVE_MEMSTREAM)
    message (STATUS "Will use MemstreamInterface instead")
    add_definitions (-DNO_MEMSTREAM)
endif (NOT HAVE_MEMSTREAM)

# set the build directory
set (PROJECT_BINARY_DIR "${PROJECT_SOURCE_DIR}/cbuild")

# default install directory set to pwd
set (CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}")

# build shared libraries
set (BUILD_SHARED_LIBS ON)

# change the build type
set (CMAKE_BUILD_TYPE "Debug")

##################################################

set (LICE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/CouchDBInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/JSON.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/Lice.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/MemstreamInterface.hh
)

set (LICE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/CouchDBInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/JSON.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Lice.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/MemstreamInterface.cc
)

##################################################

add_library (lice ${LICE_SOURCEFILES})
set_target_properties (lice PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib)
target_link_libraries (lice ${LIBS})

install (FILES ${LICE_HEADERFILES}  DESTINATION include)
install (TARGETS lice  DESTINATION lib)

##################################################

add_executable (getP8Sensor ${CMAKE_CURRENT_SOURCE_DIR}/getP8Sensor.cc)
target_link_libraries (getP8Sensor lice)

install (TARGETS getP8Sensor
         DESTINATION bin)
