project(network)

cmake_minimum_required(VERSION 2.4)
include(TestBigEndian)

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
set(LIVE555_DIR ${COMMON_DIR}/live555)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(VSOFTS_SDK_DIR ${COMMON_DIR}/vanguard/linux/v4sdk-4.6)
else()
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
        set(VSOFTS_SDK_DIR ${COMMON_DIR}/vanguard/mac/v4sdk-4.6)
    endif()
endif()

include_directories(${NEURON_COMMON_DIR}/threading/NeuronBasics)
include_directories(${VSOFTS_SDK_DIR}/inc)
include_directories(${LIVE555_DIR}/UsageEnvironment/include)
include_directories(${LIVE555_DIR}/BasicUsageEnvironment/include)
include_directories(${LIVE555_DIR}/groupsock/include)
include_directories(${LIVE555_DIR}/liveMedia/include)
include_directories(../capture)
include_directories(../codec)

set(SOURCES nlrtp.cpp)
list(APPEND SOURCES
     nlrtpstream.cpp
     nlvrtptransport.cpp
     nlrtspserver.cpp)

test_big_endian(BIG_ENDIAN)
if(${BIG_ENDIAN})
    add_definitions(-DBIG_ENDIAN)
else()
    add_definitions(-DLITTLE_ENDIAN)
endif()

add_definitions(-DBUILD_DEBUG)

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

add_library(network ${SOURCES})
