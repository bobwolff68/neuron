###################

PROJECT( neuroncommon ) # if you want to

cmake_minimum_required(VERSION 2.4)

###################
###################

set(SOURCES
   threading/NeuronBasics/ThreadMultiple.cpp
   eventhandling/eventhandler.cpp
   anyoption/anyoption.cpp
   testjigsupport.cpp
   )

###################
###################

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ../bin
        CACHE PATH
        "Single output for all executables"
        )

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

INCLUDE_DIRECTORIES( ../include ../neuroncommon )

add_definitions(-g3)
message(STATUS "cmake_system_processor is: ${CMAKE_SYSTEM_PROCESSOR}")

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
#  SET (COMPILE_FLAGS "-m32 -arch i386")
#  SET (CMAKE_EXE_LINKER_FLAGS "-m32 -arch i386")
else()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  add_definitions(-DRTI_64BIT)
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
endif()


ADD_LIBRARY(neuroncommon ${SOURCES})

find_library(PT_LIB pthread)
ADD_EXECUTABLE(test1 EXCLUDE_FROM_ALL threading/NeuronBasics/testthreads.cpp)
TARGET_LINK_LIBRARIES(test1 neuroncommon ${PT_LIB})

###################

