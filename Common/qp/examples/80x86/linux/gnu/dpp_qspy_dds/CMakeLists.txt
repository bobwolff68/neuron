
###################

PROJECT(dpp) # project name

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

set(QPDIR ../../../../../)
set(NEURONDIR ../../../../../../../Neuron/code)

INCLUDE_DIRECTORIES(. ${QPDIR}/ports/80x86/linux/gnu ${QPDIR}/include ${QPDIR}/tools/qspylogger_dds/qsddslogidl)
INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds ${QPDIR}/tools/qspylogger_dds)

add_definitions(-DRTI_LINUX)
add_definitions(-DRTI_UNIX)
add_definitions(-ggdb)
add_definitions(-DQ_SPY)

find_library(LIBQEP  libqep_spy.a ${NEURONDIR}/lib)#${QPDIR}/ports/80x86/linux/gnu/spy)
find_library(LIBQF   libqf_spy.a  ${NEURONDIR}/lib)#${QPDIR}/ports/80x86/linux/gnu/spy)
find_library(LIBQS   libqs.a  ${NEURONDIR}/lib)#${QPDIR}/ports/80x86/linux/gnu/spy)
find_library(LIBQSDDSLOGGER libqsddslogger.a ${QPDIR}/tools/qspylogger_dds/lib)

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
   dpp                                          # build executable foo
   main.cpp
   bsp.cpp			# from these sources
   philo.cpp
   table.cpp
)

TARGET_LINK_LIBRARIES(
   dpp                      # to foo link
   ${LIBQSDDSLOGGER}
   ${NDDSCPP}
   ${NDDSC}
   ${NDDSCORE}
   ${LIBQF}
   ${LIBQEP}
   ${LIBQS}
   -lpthread 
   -ldl
   -lrt
)

###################

