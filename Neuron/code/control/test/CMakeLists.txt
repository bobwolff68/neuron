##############

PROJECT( test ) # if you want to

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  SET (COMPILE_FLAGS "-m32 -arch i386")
  SET (CMAKE_EXE_LINKER_FLAGS "-m32 -arch i386")
  find_library(NDDSCPP libnddscppz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSC libnddscz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSCORE libnddscorez.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
else()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  find_library(NDDSCPP libnddscppz.a $ENV{NDDSHOME}/lib/x64Linux2.6gcc4.1.1)
  find_library(NDDSC libnddscz.a $ENV{NDDSHOME}/lib/x64Linux2.6gcc4.1.1)
  find_library(NDDSCORE libnddscorez.a $ENV{NDDSHOME}/lib/x64Linux2.6gcc4.1.1)
  add_definitions(-DRTI_64BIT)
  SET (COMPILE_FLAGS 
        "-m64"
        CACHE PATH
        "Specific for compile style")
else()
  find_library(NDDSCPP libnddscppz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSC libnddscz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSCORE libnddscorez.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  SET (COMPILE_FLAGS
        "-m32"
        CACHE PATH
        "Specific for compile style")
endif()
endif()

###################
###################

set(SOURCES_CONTROLLER controller.cpp)
set(SOURCES_SF sf.cpp)
set(SOURCES_SL sl.cpp)

###################
###################

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        bin
        CACHE PATH
        "Single output for all executables"
        )

INCLUDE_DIRECTORIES( ../../neuroncommon ../include ../../include .. ../idl)
INCLUDE_DIRECTORIES( $ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds )

foreach(cp scp ecp lscp acp)
INCLUDE_DIRECTORIES(../${cp}/idl)
INCLUDE_DIRECTORIES(../${cp}/)
endforeach()

add_definitions(-DRTI_LINUX)
add_definitions(-DRTI_UNIX)
add_definitions(-g3)

message(STATUS "cmake_system_processor is: ${CMAKE_SYSTEM_PROCESSOR}")

source_group(Qos USER_QOS_PROFILES.xml)

find_library(PT_LIB pthread)
find_library(COMMON libneuroncommon.a ../../lib)
find_library(CONTROL libcontrolplane.a  ../../lib)
find_library(READLINE readline)

ADD_CUSTOM_TARGET(Resources SOURCES USER_QOS_PROFILES.xml)
ADD_EXECUTABLE(controller ${SOURCES_CONTROLLER})
TARGET_LINK_LIBRARIES(controller ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB} ${READLINE} -ldl -lrt)

ADD_EXECUTABLE(sf ${SOURCES_SF})
TARGET_LINK_LIBRARIES(sf ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB} -ldl -lrt)

ADD_EXECUTABLE(sl ${SOURCES_SL})
TARGET_LINK_LIBRARIES(sl ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB} -ldl -lrt)

###################

