
###################

PROJECT(sftest) # project name

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        bin
        CACHE PATH
        "Single output for all executables"
        )

INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds ../../../include)
INCLUDE_DIRECTORIES(. ../../../tools/ubrain/ ../ ../../../neuroncommon)

add_definitions(-DRTI_LINUX)
add_definitions(-DRTI_UNIX)
add_definitions(-DSKIP_DDS)
add_definitions(-DUNIT_TEST_MODE)

find_library(CPPUNIT libcppunit.a)
find_library(LIBCURL libcurl.a ../../../neuroncommon/netlib)

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
   sftest                 # build executable foo
   sessionfactorytest.cpp
   ../sessionfactory.cpp
   ../../../tools/ubrain/registration.cpp
   NeuronCppUnitRunTests.cpp
)

TARGET_LINK_LIBRARIES(
   sftest                            # to foo link
   ${NDDSCPP}
   ${NDDSC}
   ${NDDSCORE}
   ${CPPUNIT}
   ${LIBCURL}
   -lidn
   -lssl
   -ldl
   -lrt
)
