
###################

PROJECT(demoendpoint) # project name

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_library(PT_LIB pthread)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ../../bin
        CACHE PATH
        "Single output for all executables"
        )

FIND_PACKAGE(PkgConfig)

pkg_search_module(LIBAVCODEC libavcodec)
pkg_search_module(LIBAVFORMAT libavformat)
pkg_search_module(LIBAVUTIL libavutil)
pkg_search_module(LIBSWSCALE libswscale)
pkg_search_module(GTK gtk+-2.0)

LINK_DIRECTORIES(LIBAVCODEC_LIBDIR LIBAVFORMAT_LIBDIR LIBAVUTIL_LIBDIR LIBSWSCALE_LIBDIR GTK_LIBDIR)
LINK_DIRECTORIES(LIBAVCODEC_LIBRARY_DIRS LIBAVFORMAT_LIBRARY_DIRS LIBAVUTIL_LIBRARY_DIRS LIBSWSCALE_LIBRARY_DIRS GTK_LIBRARY_DIRS)


INCLUDE_DIRECTORIES(../../include ../../sf ../../sl ../../neuroncommon ../../sl ../../control/include)
INCLUDE_DIRECTORIES(../../control ../../control/scp ../../control/lscp ../../control/acp ../../control/ecp)
INCLUDE_DIRECTORIES(../../control/idl ../../control/scp/idl ../../control/lscp/idl ../../control/acp/idl ../../control/ecp/idl)
INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds)
INCLUDE_DIRECTORIES(../../media ../../media/H264 ../../media/idl ../../entities ../../neuroncommon ../ubrain)
INCLUDE_DIRECTORIES(/usr/include/SDL ${GTK_INCLUDEDIR} ${GTK_INCLUDE_DIRS})

message("GTK: ${GTK_LIBRARIES}")

add_definitions(-DRTI_LINUX)
add_definitions(-DRTI_UNIX)
add_definitions(-DVERBOSE_OUTPUT)
add_definitions(-D_GNU_SOURCE=1)
add_definitions(-D_REENTRANT)
add_definitions(-DSF_ENDPOINT_MODE)
add_definitions(-g3)

#add_definitions(-m64)
#add_definitions(-DRTI_64BIT)

find_library(COMMON libneuroncommon.a ../../lib)
find_library(CONTROL libcontrolplane.a ../../lib)
find_library(NDDSCPP libnddscppz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
find_library(NDDSC libnddscz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
find_library(NDDSCORE libnddscorez.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
find_library(LIBAVCODEC_LIBRARY libavcodec.a)
find_library(LIBAVFORMAT_LIBRARY libavformat.a)
find_library(LIBAVUTIL_LIBRARY libavutil.a)
find_library(LIBSWSCALE_LIBRARY libswscale.a)
find_library(LIBCURL libcurl.a ../../neuroncommon/netlib)

ADD_EXECUTABLE(
   demoendpoint                                          # build executable foo
   DemoEndpoint.cpp
   ../ubrain/registration.cpp
   ../../sf/sessionfactory.cpp			# from these sources
   ../../sl/sessionleader.cpp
   ../../media/idl/media.cxx
   ../../media/idl/mediaSupport.cxx
   ../../media/idl/mediaPlugin.cxx
)

TARGET_LINK_LIBRARIES(
   demoendpoint                            # to foo link
   ${COMMON}                  # library bar which is in vendor/bar
   ${CONTROL}
   ${NDDSCPP}
   ${NDDSC}
   ${NDDSCORE}
   ${PT_LIB}
   ${LIBCURL}
   -lSDL
   ${LIBSWSCALE_LIBRARY}
   ${LIBAVCODEC_LIBRARY}
   ${LIBAVFORMAT_LIBRARY}
   ${LIBAVCODEC_LIBRARY}
   ${LIBAVUTIL_LIBRARY}
   ${GTK_LIBRARIES} 
   -ldl
   -lrt
   -lm
   -lz
)

###################

