project(network)

cmake_minimum_required(VERSION 2.4)
include(TestBigEndian)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    bin
    CACHE PATH
    "Single output for all executables")

set(NEURON_CODE_DIR ../../../..)
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
include_directories(${NEURON_CODE_DIR}/neuronlight/capture)
include_directories(${NEURON_CODE_DIR}/neuronlight/codec)
include_directories(${NEURON_CODE_DIR}/neuronlight/network)
include_directories(${LIVE555_DIR}/UsageEnvironment/include)
include_directories(${LIVE555_DIR}/BasicUsageEnvironment/include)
include_directories(${LIVE555_DIR}/groupsock/include)
include_directories(${LIVE555_DIR}/liveMedia/include)


find_library(CAPTURE libcapture.a ${NEURON_CODE_DIR}/lib)
find_library(CODEC libcodec.a ${NEURON_CODE_DIR}/lib)
find_library(NETWORK libnetwork.a ${NEURON_CODE_DIR}/lib)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_library(VSOFTS libv4codecsdk-l32i-all-mt.a ${VSOFTS_SDK_DIR}/lib)
    find_library(IRC libirc.a ${VSOFTS_SDK_DIR}/../lib-icc32)
    find_library(IMF libimf.a ${VSOFTS_SDK_DIR}/../lib-icc32)
    find_library(SVML libsvml.a ${VSOFTS_SDK_DIR}/../lib-icc32)
    find_library(UENV libUsageEnvironment.a ${LIVE555_DIR}/UsageEnvironment)
    find_library(BUENV libBasicUsageEnvironment.a ${LIVE555_DIR}/BasicUsageEnvironment)
    find_library(GSOCK libgroupsock.a ${LIVE555_DIR}/groupsock)
    find_library(LIVEMEDIA libliveMedia.a ${LIVE555_DIR}/liveMedia)
else()
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
        find_library(VSOFTS libv4codecsdk-m32i-all-mt.a ${VSOFTS_SDK_DIR}/lib)
    endif()
endif()

set(SOURCES main.cpp)
list(APPEND SOURCES
     nlrtcamstream.cpp)
     
test_big_endian(BIG_ENDIAN)
if(${BIG_ENDIAN})
    add_definitions(-DBIG_ENDIAN)
else()
    add_definitions(-DLITTLE_ENDIAN)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(COMPILE_FLAGS "-m32 -arch i386")
    set(CMAKE_EXE_LINKER_FLAGS "-m32 -arch i386")
else()
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
# Note this is a SPECIAL for Vanguard which requires 32-bit compilation currently 8/2011 and linking on 64-bit machines.
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
	    set(CMAKE_EXE_LINKER_FLAGS "-m32")
            set(COMPILE_FLAGS "-m32" CACHE PATH "Specific for compile style")
        endif()
    endif()
endif()

add_definitions(-DSOCKLEN_T=socklen_t)
add_definitions(-D_LARGEFILE_SOURCE=1)
add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-DBUILD_DEBUG)

add_executable(
    rtcamstream 
    ${SOURCES}
)

target_link_libraries(
    rtcamstream
    ${CAPTURE}
    ${CODEC}
    ${NETWORK}
    ${LIVEMEDIA}
    ${GSOCK}
    ${BUENV}
    ${UENV}
    ${VSOFTS}
    ${IRC}
    ${IMF}
    ${SVML}
    -lm
    -lpthread
)
