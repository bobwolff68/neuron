
###################

PROJECT(ubrain) # project name

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ../../bin
        CACHE PATH
        "Single output for all executables"
        )

SET (SOURCES ubrain.cpp 
   parsecmd.cpp
   sshmgmt.cpp
   shell.cpp
   registration.cpp
   serverbase.cpp
   localitems.cpp
   xmlinout.cpp
   regserver.cpp
   ubrainmanager.cpp
   controller.cpp
)

# Hack for survival at the moment...just find and link the library. No true dependencies yet.
find_library(COMMON libneuroncommon.a ../../lib)
find_library(CONTROL libcontrolplane.a ../../lib)

find_library(PT_LIB pthread)

# Big hack for temporarily binding libcurl into our program prior to libcurl being part of neuroncommon
find_library(LIBCURL libcurl.a ../../neuroncommon/netlib)
find_library(LIBXML2 libxml2.a)
find_library(LIBC libc.a)
find_library(LIBM libm.a)
find_library(READLINE readline)

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

INCLUDE_DIRECTORIES( ../../include ../../neuroncommon ../../entities ) # add to include path
INCLUDE_DIRECTORIES( ../../control ../../control/include ../../control/idl ) # add to include path

INCLUDE_DIRECTORIES( /usr/include/libxml2 )

INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds)

foreach(cp scp ecp lscp acp)
INCLUDE_DIRECTORIES(../../control/${cp}/idl)
INCLUDE_DIRECTORIES(../../control/${cp}/)
endforeach()

#add_definitions(-DSKIP_DDS -g3 -DDEBUG -D_DEBUG)
add_definitions(-g3 -DDEBUG -D_DEBUG)
add_definitions(-DRTI_LINUX -DRTI_UNIX)
add_definitions(-DUBRAIN)

ADD_EXECUTABLE(ubrain ${SOURCES})

TARGET_LINK_LIBRARIES(
   ubrain                            # to foo link
   ${CONTROL} ${COMMON} ${NDDSCPP} ${NDDSC} ${NDDSCORE}
   ${LIBXML2}
   ${LIBCURL}		# Temporarily necessary
   ${READLINE}
   ${LIBC} ${LIBM} ${PT_LIB}
   -lncurses
   -lidn -lssl -ldl -lrt
)

###################

