
###################

PROJECT(ubrain) # project name

cmake_minimum_required(VERSION 2.4)

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
)

#ADD_SUBDIRECTORY( ../../control ) # add subdir
#ADD_SUBDIRECTORY( ../../neuroncommon ) # add subdir

# Hack for survival at the moment...just find and link the library. No true dependencies yet.
find_library(COMMON libneuroncommon.a ../../lib)

find_library(PT_LIB pthread)

# Big hack for temporarily binding libcurl into our program prior to libcurl being part of neuroncommon
find_library(LIBCURL libcurl.a ../../neuroncommon/netlib)
find_library(LIBXML2 libxml2.a)
find_library(LIBC libc.a)
find_library(LIBM libm.a)
find_library(LIBZ libz.a)

INCLUDE_DIRECTORIES( ../../include ../../neuroncommon ) # add to include path

INCLUDE_DIRECTORIES( /usr/include/libxml2 )

#INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds)

add_definitions(-DSKIP_DDS -g3 -DDEBUG -D_DEBUG)

ADD_EXECUTABLE(ubrain ${SOURCES})

TARGET_LINK_LIBRARIES(
   ubrain                            # to foo link
#   controlplane                  # library bar which is in vendor/bar
   ${COMMON}		# neuroncommon
   ${LIBXML2}
   ${LIBCURL}		# Temporarily necessary
   ${LIBC} ${LIBM} ${LIBZ} ${PT_LIB}
)

###################

