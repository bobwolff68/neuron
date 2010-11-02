###################

PROJECT( controlplane ) # if you want to

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

ADD_LIBRARY(
   controlplane
   co.cpp
)

###################
