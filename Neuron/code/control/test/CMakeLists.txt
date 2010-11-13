##############

PROJECT( test ) # if you want to

cmake_minimum_required(VERSION 2.4)

###################
###################

set(SOURCES_CONTROLLER controller.cpp)
set(SOURCES_SF sf.cpp)
set(SOURCES_SL sl.cpp)
set(SOURCES_EP ep.cpp)

###################
###################

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        bin
        CACHE PATH
        "Single output for all executables"
        )

INCLUDE_DIRECTORIES( ../include ../../include .. ../idl)
INCLUDE_DIRECTORIES( $ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds )

foreach(cp scp ecp lscp acp)
INCLUDE_DIRECTORIES(../${cp}/idl)
INCLUDE_DIRECTORIES(../${cp}/)
endforeach()

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
find_library(COMMON libneuroncommon.a ../../lib)
find_library(CONTROL libcontrolplane.a  ../../lib)
find_library(READLINE readline)

ADD_EXECUTABLE(controller ${SOURCES_CONTROLLER})
TARGET_LINK_LIBRARIES(controller ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB} ${READLINE} )

ADD_EXECUTABLE(sf ${SOURCES_SF})
TARGET_LINK_LIBRARIES(sf ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB})

ADD_EXECUTABLE(sl ${SOURCES_SL})
TARGET_LINK_LIBRARIES(sl ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB})

ADD_EXECUTABLE(ep ${SOURCES_EP})
TARGET_LINK_LIBRARIES(ep ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE} ${PT_LIB})



###################

