
###################

PROJECT(rp_standalone) # project name

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_library(PT_LIB pthread)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        bin
        CACHE PATH
        "Single output for all executables"
        )

INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds ../../../)
INCLUDE_DIRECTORIES(../ ../../../../media/idl ../../../../media/entity_info_idl)
INCLUDE_DIRECTORIES(../../../../neuroncommon ../../../../media ../../../../include ../../../../media/H264)

add_definitions(-DRTI_LINUX)
add_definitions(-DRTI_UNIX)

#find_library(CPPUNIT libcppunit.so)

find_library(NEURONCOMMON libneuroncommon.a ~/workspace/sprint0-branch/lib)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  SET (COMPILE_FLAGS "-m32 -arch i386")
  SET (CMAKE_EXE_LINKER_FLAGS "-m32 -arch i386")
  find_library(NDDSCPP libnddscppz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSC libnddscz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSCORE libnddscorez.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
else()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  find_library(NDDSCPP libnddscppz.a $ENV{NDDSHOME}/lib/x64Linux2.6gcc4.1.1)
  find_library(NDDSC libnddscz.a $ENV{NDDSHOME}/lib/x64Linux2.6gcc4.1.1)
  find_library(NDDSCORE libnddscorez.a $ENV{NDDSHOME}/lib/x64Linux2.6gcc4.1.1)
  add_definitions(-DRTI_64BIT)
  SET (COMPILE_FLAGS 
        "-m64"
        CACHE PATH
        "Specific for compile style")
else()
  find_library(NDDSCPP libnddscppz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSC libnddscz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSCORE libnddscorez.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  SET (COMPILE_FLAGS
        "-m32"
        CACHE PATH
        "Specific for compile style")
endif()
endif()

ADD_EXECUTABLE(
   rp_standalone
   rp_standalone.cpp                 # build executable foo
   ../../../../media/idl/media.cxx
   ../../../../media/idl/mediaSupport.cxx
   ../../../../media/idl/mediaPlugin.cxx
   ../../../../media/entity_info_idl/entityinfo.cxx
   ../../../../media/entity_info_idl/entityinfoSupport.cxx
   ../../../../media/entity_info_idl/entityinfoPlugin.cxx
)

TARGET_LINK_LIBRARIES(
   rp_standalone                           # to foo link
   ${NEURONCOMMON}
   ${NDDSCPP}
   ${NDDSC}
   ${NDDSCORE}
#   ${CPPUNIT}
   -ldl
   -lrt
)

