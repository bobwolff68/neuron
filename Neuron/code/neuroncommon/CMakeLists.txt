###################

PROJECT( neuroncommon ) # if you want to

cmake_minimum_required(VERSION 2.4)

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ../lib
        CACHE PATH
        "Single output for all libraries"
        )

SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        ../lib
        CACHE PATH
        "Single output for all libraries"
        )

# INCLUDE_DIRECTORIES( ../cminc )

message(STATUS "cmake_system_processor is: ${CMAKE_SYSTEM_PROCESSOR}")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  SET (COMPILE_FLAGS 
        "-m64"
        CACHE PATH
        "Specific for compile style")
else()
  SET (COMPILE_FLAGS
        "-m32"
        CACHE PATH
        "Specific for compile style")
endif()

ADD_LIBRARY(
   neuroncommon
   threading/NeuronBasics/ThreadMultiple.cpp
)

###################

