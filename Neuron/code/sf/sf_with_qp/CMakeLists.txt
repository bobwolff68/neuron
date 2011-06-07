
###################

PROJECT(sf_with_qp) # project name

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

set(QPDIR ../../../../Common/qp)

INCLUDE_DIRECTORIES(. ../../include qmcode/ ../../neuroncommon ${QPDIR}/ports/80x86/linux/gnu)# ../sl ../control/include)
INCLUDE_DIRECTORIES(${QPDIR}/include ../../tools/ubrain ${QPDIR}/tools/qspylogger_dds ${QPDIR}/tools/qspylogger_dds/qsddslogidl)
#INCLUDE_DIRECTORIES(../control ../control/scp ../control/lscp ../control/acp ../control/ecp)
#INCLUDE_DIRECTORIES(../control/idl ../control/scp/idl ../control/lscp/idl ../control/acp/idl ../control/ecp/idl)
INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds)
#INCLUDE_DIRECTORIES(../media ../media/H264 ../media/idl ../entities ../neuroncommon)# ../tools/DemoEndpoint)
#INCLUDE_DIRECTORIES(../tools/ubrain ../media/entity_info_idl)

add_definitions(-DRTI_LINUX)
add_definitions(-DRTI_UNIX)
#add_definitions(-DVERBOSE_OUTPUT)
#add_definitions(-D_GNU_SOURCE=1)
#add_definitions(-D_REENTRANT)
add_definitions(-ggdb)
#add_definitions(-DSKIP_DDS)
add_definitions(-DQ_SPY)
#add_definitions(-g3)
# for speed... add_definitions(-O3)

#find_library(COMMON libneuroncommon.a ../../lib)
#find_library(CONTROL libcontrolplane.a ../lib)
find_library(LIBQSDDSLOGGER libqsddslogger.a ${QPDIR}/tools/qspylogger_dds/lib)
find_library(LIBCURL libcurl.a ../../neuroncommon/netlib)
find_library(LIBQEP  libqep_spy.a  ../../lib)
find_library(LIBQF   libqf_spy.a   ../../lib)
find_library(LIBQS   libqs.a   ../../lib)

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
   sf                                          # build executable foo
   sessionfactorymain.cpp
   sessionfactoryprocess.cpp			# from these sources
   sessionfactory.cpp
   callbacks.cpp
   ../../tools/ubrain/registration.cpp
   qmcode/sfsmregistration.cpp
)

TARGET_LINK_LIBRARIES(
   sf                            # to foo link
#   ${COMMON}                  # library bar which is in vendor/bar
#   ${CONTROL}
   ${LIBQSDDSLOGGER}
   ${NDDSCPP}
   ${NDDSC}
   ${NDDSCORE}
   ${LIBQF}
   ${LIBQEP}
   ${LIBQS}
   ${LIBCURL}
   ${PT_LIB}
   -lidn
   -lssl
   -ldl
   -lrt
)

###################

