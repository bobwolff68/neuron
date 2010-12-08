
###################

PROJECT(sf) # project name

cmake_minimum_required(VERSION 2.4)
find_library(PT_LIB pthread)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ../bin
        CACHE PATH
        "Single output for all executables"
        )

INCLUDE_DIRECTORIES(../include ../neuroncommon ../sl ../control/include)
INCLUDE_DIRECTORIES(../control ../control/scp ../control/lscp ../control/acp ../control/ecp)
INCLUDE_DIRECTORIES(../control/idl ../control/scp/idl ../control/lscp/idl ../control/acp/idl ../control/ecp/idl)
INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds)
INCLUDE_DIRECTORIES(../media ../media/idl ../entities ../neuroncommon/eventhandling ../neuroncommon/threading/NeuronBasics)

add_definitions(-DRTI_LINUX)
add_definitions(-DRTI_UNIX)
add_definitions(-DVERBOSE_OUTPUT)
#add_definitions(-g3)

find_library(COMMON libneuroncommon.a ../lib)
find_library(CONTROL libcontrolplane.a ../lib)
find_library(NDDSCPP libnddscpp.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
find_library(NDDSC libnddsc.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
find_library(NDDSCORE libnddscore.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
#ADD_SUBDIRECTORY( ../control ) # add subdir
#ADD_SUBDIRECTORY( ../neuroncommon ) # add subdir

# INCLUDE_DIRECTORIES( ../cminc ) # add to include path

ADD_EXECUTABLE(
   sf                                          # build executable foo
   sessionfactory.cpp			# from these sources
   sf.cpp
   ../sl/sessionleader.cpp
   ../media/idl/media.cxx
   ../media/idl/mediaSupport.cxx
   ../media/idl/mediaPlugin.cxx
#   ../control/CPInterface.cpp
)

TARGET_LINK_LIBRARIES(
   sf                            # to foo link
   ${COMMON}                  # library bar which is in vendor/bar
   ${CONTROL}
   ${NDDSCPP}
   ${NDDSC}
   ${NDDSCORE}
   ${PT_LIB}
   -ldl
   -lrt
)

###################

