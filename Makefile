# Makefile to build class 'midicycle' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name
lib.name = midicycle

# input source file (class name == source file basename)
class.sources = midicycle.c

common.sources = seq/seq.cpp

# all extra files to be included in binary distribution of the library
datafiles = readme

cflags = -std=c++14
CC = g++


# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder


    