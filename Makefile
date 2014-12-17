# Makefile for testhello pass

# Path to top level of LLVM hierarchy
LEVEL = ../../..

# Name of the library to build
LIBRARYNAME = ProgramSlicing

# Make the shared library become a loadable module so the tools can
# dlopen/dlsym on the resulting library.
LOADABLE_MODULE = 1

#USEDLIBS = anders

#LLVM_OBJ_ROOT = /v/filer4b/v20q001/yychen/LLVM/llvm

# Include the makefile implementation stuff
include $(LEVEL)/Makefile.common

#LIBS += /usr/local/lib/anders-aa.so
LIBS += /usr/local/lib/libAnders.so


# LDFLAGS += -lPointerAnalysis
