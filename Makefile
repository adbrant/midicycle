# Makefile to build class 'midicycle' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name
lib.name = midicycle

# input source file (class name == source file basename)
class.sources = src/midicycle.cpp src/multicycle.cpp

common.sources = src/SeqRecorder.cpp 

# all extra files to be included in binary distribution of the library
datafiles = readme

cflags = -std=c++14 -Wno-unused-parameter
CC = g++


# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder


test: ./src/*.cpp ./src/*.hpp
	g++ -std=c++14 -DTEST -Isrc src/SeqRecorder.cpp -o test 
	./test | grep 'Done'
	