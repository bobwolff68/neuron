
###################

project(qsddslogger)

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY 
    lib 
    CACHE PATH 
    "Single output for all executables")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY 
    lib 
    CACHE PATH 
    "Single output for all executables")

include_directories(. $ENV{HOME}/workspace/gitneuron/Common/qp/ports/80x86/linux/gnu)
include_directories($ENV{HOME}/workspace/gitneuron/Common/qp/include)
include_directories($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds qsddslogidl)

add_definitions(-DRTI_LINUX)
add_definitions(-DRTI_UNIX)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(COMPILE_FLAGS "-m32 -arch i386")
  set(CMAKE_EXE_LINKER_FLAGS "-m32 -arch i386")
  find_library(NDDSCPP libnddscppz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSC libnddscz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSCORE libnddscorez.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
else()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  find_library(NDDSCPP libnddscppz.a $ENV{NDDSHOME}/lib/x64Linux2.6gcc4.1.1)
  find_library(NDDSC libnddscz.a $ENV{NDDSHOME}/lib/x64Linux2.6gcc4.1.1)
  find_library(NDDSCORE libnddscorez.a $ENV{NDDSHOME}/lib/x64Linux2.6gcc4.1.1)
  add_definitions(-DRTI_64BIT)
  set(COMPILE_FLAGS 
      "-m64"
      CACHE PATH
      "Specific for compile style")
else()
  find_library(NDDSCPP libnddscppz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSC libnddscz.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  find_library(NDDSCORE libnddscorez.a $ENV{NDDSHOME}/lib/i86Linux2.6gcc4.1.1)
  set(COMPILE_FLAGS
      "-m32"
      CACHE PATH
      "Specific for compile style")
endif()
endif()

set(SOURCES qsddslogger.cpp)
set(idl qsddslog)

add_custom_command(
    OUTPUT ${idl}idl/${idl}.cxx ${idl}idl/${idl}.h ${idl}idl/${idl}Support.cxx ${idl}idl/${idl}Support.h ${idl}idl/${idl}Plugin.cxx ${idl}idl/${idl}Plugin.h
    COMMAND $ENV{NDDSHOME}/scripts/rtiddsgen -replace -namespace ${CMAKE_CURRENT_SOURCE_DIR}/${idl}idl/${idl}.idl
    DEPENDS $ENV{NDDSHOME}/scripts/rtiddsgen ${CMAKE_CURRENT_SOURCE_DIR}/${idl}idl/${idl}.idl
)

list(APPEND SOURCES qsddslogreader.cpp)
list(APPEND SOURCES ${idl}idl/${idl}.cxx ${idl}idl/${idl}Support.cxx ${idl}idl/${idl}Plugin.cxx)

add_library(
   qsddslogger
   ${SOURCES}
)

target_link_libraries(
   qsddslogger
   ${NDDSCPP}
   ${NDDSC}
   ${NDDSCORE}
   -ldl
   -lrt
)

###################

