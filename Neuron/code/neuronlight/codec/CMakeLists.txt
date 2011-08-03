project(codec)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    lib
    CACHE PATH
    "Single output for all libraries")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    lib
    CACHE PATH
    "Single output for all libraries")

set(NEURON_COMMON_DIR ../../neuroncommon)
set(VSOFTS_SDK_DIR $ENV{HOME}/workspace/vsofts/h264sdk/v4sdk-4.6/)
include_directories(${NEURON_COMMON_DIR}/threading/NeuronBasics)
include_directories(${VSOFTS_SDK_DIR}/inc)
include_directories(../capture)

set(SOURCES v4rtenc.cpp)

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

