
###################

PROJECT(sf) # project name

cmake_minimum_required(VERSION 2.4)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ../bin
        CACHE PATH
        "Single output for all executables"
        )

ADD_SUBDIRECTORY( ../control ) # add subdir

# INCLUDE_DIRECTORIES( ../cminc ) # add to include path

ADD_EXECUTABLE(
   sf                                          # build executable foo
   sf.cpp			# from these sources
   sfactory.cpp
)

TARGET_LINK_LIBRARIES(
   sf                            # to foo link
   controlplane                  # library bar which is in vendor/bar
)

###################

