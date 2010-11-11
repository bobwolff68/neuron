
###################

PROJECT(ubrain) # project name

cmake_minimum_required(VERSION 2.4)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ../../bin
        CACHE PATH
        "Single output for all executables"
        )

#ADD_SUBDIRECTORY( ../../control ) # add subdir
#ADD_SUBDIRECTORY( ../../neuroncommon ) # add subdir

INCLUDE_DIRECTORIES( ../../include ../../neuroncommon ) # add to include path
#INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds)

add_definitions(-DSKIP_DDS)

ADD_EXECUTABLE(
   ubrain
   ubrain.cpp
   parsecmd.cpp
   sshmgmt.cpp
   shell.cpp
)

TARGET_LINK_LIBRARIES(
   ubrain                            # to foo link
#   controlplane                  # library bar which is in vendor/bar
   neuroncommon
)

###################

