
###################

PROJECT(luaQuickProof) # project name

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ../../bin
        CACHE PATH
        "Single output for all executables"
        )

SET (SOURCES LuaQuickProof.cpp 
    TestObject.cpp
    TestingSamples.cpp
    NeuronCppUnitRunTests.cpp
)

# Hack for survival at the moment...just find and link the library. No true dependencies yet.
find_library(COMMON libneuroncommon.a ../../lib)
find_library(CONTROL libcontrolplane.a ../../lib)

find_library(PT_LIB pthread)

find_library(LIBC libc.a)
find_library(LIBM libm.a)

find_library(LIBCPPUNIT_SO libcppunit.so)

# Lua and SWIG
set(SWIG_SOURCES LuaInterface.i)
foreach(i_file ${SWIG_SOURCES})
    string(REGEX REPLACE "\\.i" ""  basename ${i_file})
    add_custom_command(
        OUTPUT ${basename}_wrap.cxx
        COMMAND swig -c++ -lua ${CMAKE_CURRENT_SOURCE_DIR}/${i_file}
	    DEPENDS /usr/bin/swig ${CMAKE_CURRENT_SOURCE_DIR}/${i_file}
    )
    list (APPEND SOURCES ${basename}_wrap.cxx)
endforeach()

find_library(LIBLUA liblua5.1.a)
INCLUDE_DIRECTORIES( /usr/include/lua5.1 ) # add to include path

INCLUDE_DIRECTORIES( ../../include ../../neuroncommon ../../entities ) # add to include path
INCLUDE_DIRECTORIES( ../../control ../../control/include ../../control/idl ) # add to include path

#add_definitions(-DSKIP_DDS -g3 -DDEBUG -D_DEBUG)
add_definitions(-g3 -DDEBUG -D_DEBUG)

ADD_EXECUTABLE(luatest ${SOURCES})

TARGET_LINK_LIBRARIES(
   luatest                            # to foo link
   ${COMMON}
   ${LIBLUA}
   ${LIBCPPUNIT_SO}
   ${LIBC} ${LIBM} ${PT_LIB}
   -lncurses
   -lidn -lssl -ldl -lrt
)

###################

