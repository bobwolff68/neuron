###################

PROJECT( scp_test ) # if you want to

cmake_minimum_required(VERSION 2.4)

###################
###################

set(SOURCES_MASTER scp_master.cpp)
set(SOURCES_SLAVE scp_slave.cpp)

###################
###################

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        bin
        CACHE PATH
        "Single output for all executables"
        )

INCLUDE_DIRECTORIES( .. ../idl ../../idl ../.. ../../../include ../../../neuroncommon)
INCLUDE_DIRECTORIES( $ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds )
add_definitions(-DRTI_DARWIN)
add_definitions(-DRTI_UNIX)
add_definitions(-m32)
add_definitions(-arch i386)

message(STATUS "cmake_system_processor is: ${CMAKE_SYSTEM_PROCESSOR}")

  SET (COMPILE_FLAGS "-m32 -arch i386")
  SET (CMAKE_EXE_LINKER_FLAGS "-m32 -arch i386")


find_library(PT_LIB pthread)
find_library(NDDSCPP libnddscppzd.a $ENV{NDDSHOME}/lib/i86Darwin10gcc4.2.1)
find_library(NDDSC libnddsczd.a $ENV{NDDSHOME}/lib/i86Darwin10gcc4.2.1)
find_library(NDDSCORE libnddscorezd.a $ENV{NDDSHOME}/lib/i86Darwin10gcc4.2.1)
find_library(COMMON libneuroncommon.a ../../../lib)
find_library(CONTROL libcontrolplane.a  ../../../lib)

ADD_EXECUTABLE(scp_master scp_master.cpp)
TARGET_LINK_LIBRARIES(scp_master ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB})

ADD_EXECUTABLE(scp_slave scp_slave.cpp)
TARGET_LINK_LIBRARIES(scp_slave ${CONTROL} i${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB})


###################

