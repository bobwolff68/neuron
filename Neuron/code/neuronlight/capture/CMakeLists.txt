project(capture)
cmake_minimum_required(VERSION 2.4)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    lib
    CACHE PATH
    "Single output for all libraries")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    lib
    CACHE PATH
    "Single output for all libraries")

set(NEURON_COMMON_DIR ../../neuroncommon)
include_directories(${NEURON_COMMON_DIR}/threading/NeuronBasics)
set(SOURCES)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(COMPILE_FLAGS "-m32 -arch i386")
    set(CMAKE_EXE_LINKER_FLAGS "-m32 -arch i386")
else()
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        list(APPEND SOURCES V4L2Cap.cpp)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            set(COMPILE_FLAGS "-m64" CACHE PATH "Specific for compile style")
        else()
            set(COMPILE_FLAGS "-m32" CACHE PATH "Specific for compile style")
        endif()
    endif()
endif()

add_library(capture ${SOURCES})
