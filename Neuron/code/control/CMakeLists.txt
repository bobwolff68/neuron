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

# ADD_SUBDIRECTORY(scp)

# INCLUDE_DIRECTORIES( ../cminc )

INCLUDE_DIRECTORIES(. ../include ../neuroncommon idl scp/idl)
INCLUDE_DIRECTORIES($ENV{NDDSHOME}/include $ENV{NDDSHOME}/include/ndds)

#add_definitions(-DRTI_DARWIN)
#add_definitions(-DRTI_UNIX)
#add_definitions(-m32 -arch i386)

set(IDL_SOURCES idl/CPInterfaceIDL.idl scp/idl/SCPInterface.idl)

#set (CMAKE_EXE_LINKER_FLAGS "-m32 -arch i386")

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

list (APPEND SOURCES
	CPInterfaceT.h
	scp/SCPEvent.cpp
	scp/SCPInterface.cpp
	scp/SCPMaster.cpp
	scp/SCPMasterObject.cpp
	scp/SCPObject.cpp
	scp/SCPSlave.cpp
	scp/SCPSlaveObject.cpp
	scp/SCPEvent.h
	scp/SCPInterface.h
	scp/SCPMaster.h
	scp/SCPMasterObject.h
	scp/SCPObject.h
	scp/SCPSlave.h
	scp/SCPSlaveObject.h
)

ADD_LIBRARY(
   controlplane
   ${SOURCES}
)

###################
