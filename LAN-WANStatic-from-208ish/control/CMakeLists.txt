###################

PROJECT( control ) # if you want to

cmake_minimum_required(VERSION 2.4)

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
	../lib
	CACHE PATH
	"Single output for all libraries"
	)

SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
	../lib
	CACHE PATH
	"Single output for all libraries"
	)

# INCLUDE_DIRECTORIES( ../cminc )

INCLUDE_DIRECTORIES(. ../include ../neuroncommon idl)
INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds)

add_definitions(-DRTI_LINUX -DRTI_UNIX)

#add_definitions(-g3)

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

set(IDL_SOURCES idl/CPInterfaceIDL.idl)
list(APPEND IDL_SOURCES scp/idl/SCPInterface.idl)
list(APPEND IDL_SOURCES acp/idl/ACPInterface.idl)
list(APPEND IDL_SOURCES ecp/idl/ECPInterface.idl)
list(APPEND IDL_SOURCES lscp/idl/LSCPInterface.idl)

set(SOURCES)

foreach(idl_file ${IDL_SOURCES})
    string(REGEX REPLACE "\\.idl" ""  basename ${idl_file})
    add_custom_command(
        OUTPUT ${basename}.cxx ${basename}.h ${basename}Plugin.cxx ${basename}Plugin.h ${basename}Support.cxx ${basename}Support.h
        COMMAND $ENV{NDDSHOME}/scripts/rtiddsgen -I idl -language C++ -namespace -replace ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.idl
	DEPENDS $ENV{NDDSHOME}/scripts/rtiddsgen ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.idl
    )
    list (APPEND SOURCES ${basename}.cxx ${basename}Plugin.cxx ${basename}Support.cxx)
endforeach()

list(APPEND SOURCES
	CPInterfaceT.h
	CPInterface.h
	CPInterface.cpp
)

foreach(cp scp ecp lscp acp)
INCLUDE_DIRECTORIES(${cp}/idl)
string(TOUPPER ${cp} prefix) 
list(APPEND SOURCES
	${cp}/${prefix}Event.cpp
	${cp}/${prefix}Interface.cpp
	${cp}/${prefix}Master.cpp
	${cp}/${prefix}MasterObject.cpp
	${cp}/${prefix}Object.cpp
	${cp}/${prefix}Slave.cpp
	${cp}/${prefix}SlaveObject.cpp
	${cp}/${prefix}Event.h
	${cp}/${prefix}Interface.h
	${cp}/${prefix}Master.h
	${cp}/${prefix}MasterObject.h
	${cp}/${prefix}Object.h
	${cp}/${prefix}Slave.h
	${cp}/${prefix}SlaveObject.h
)
endforeach()

ADD_LIBRARY(
   controlplane
   ${SOURCES}
)

###################
