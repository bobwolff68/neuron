
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
)

#ADD_SUBDIRECTORY( ../../control ) # add subdir
#ADD_SUBDIRECTORY( ../../neuroncommon ) # add subdir

# Hack for survival at the moment...just find and link the library. No true dependencies yet.
find_library(COMMON libneuroncommon.a ../../lib)

# Big hack for temporarily binding libcurl into our program prior to libcurl being part of neuroncommon
find_library(LIBCURL libcurl.a ../../neuroncommon/netlib)

INCLUDE_DIRECTORIES( ../../include ../../neuroncommon ) # add to include path
#INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds)

add_definitions(-DSKIP_DDS -g3 -DDEBUG -D_DEBUG)

ADD_EXECUTABLE(ubrain ${SOURCES})

TARGET_LINK_LIBRARIES(
   ubrain                            # to foo link
#   controlplane                  # library bar which is in vendor/bar
   ${COMMON}		# neuroncommon
   ${LIBCURL}		# Temporarily necessary
)

###################

