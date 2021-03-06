project(codec)

cmake_minimum_required(VERSION 2.4)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    lib
    CACHE PATH
    "Single output for all libraries")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    lib
    CACHE PATH
    "Single output for all libraries")

set(NEURON_CODE_DIR ../..)
set(COMMON_DIR ${NEURON_CODE_DIR}/../../Common)
set(NEURON_COMMON_DIR ${NEURON_CODE_DIR}/neuroncommon)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(VSOFTS_SDK_DIR ${COMMON_DIR}/vanguard/linux/v4sdk-4.6)
else()
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
        set(VSOFTS_SDK_DIR ${COMMON_DIR}/vanguard/mac/v4sdk-4.6)
    endif()
endif()

include_directories(${NEURON_COMMON_DIR}/threading/NeuronBasics)
include_directories(${VSOFTS_SDK_DIR}/inc)
include_directories(../capture)

add_definitions(-DBUILD_DEBUG)

set(SOURCES v4rtenc.cpp)
list(APPEND SOURCES v4fifoout.cpp)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(COMPILE_FLAGS "-m32 -arch i386")
    set(CMAKE_EXE_LINKER_FLAGS "-m32 -arch i386")
else()
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            set(COMPILE_FLAGS "-m64" CACHE PATH "Specific for compile style")
        else()
            set(COMPILE_FLAGS "-m32" CACHE PATH "Specific for compile style")
        endif()
    endif()
endif()

add_library(codec ${SOURCES})