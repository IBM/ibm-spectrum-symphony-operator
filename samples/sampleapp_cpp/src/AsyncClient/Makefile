#
# This exposed source code is the confidential and proprietary property of
# IBM Corporation. Your right to use is strictly limited by the terms of the
# license agreement entered into with IBM Corporation. 
#
######################################################################
#
# 		Makefile for C++ API Single Library
#  ------- R E A D   M E   T O   A V O I D    F A I L U R E  --------
#
#
#
######################################################################

## load predefined macro

TOP = ${SOAM_HOME}/${SOAM_VERSION}
OUTPUT = ../Output

ifeq ($(shell uname -m) , ppc64)
BIT64_FLAG = -m64
endif

CC = g++ $(BIT64_FLAG) -Wno-deprecated -Wall -pthread
#CC = icc -gcc-version=346
#set the correct gcc version installed on your platform (for example for RHEL6 set -gcc-version=446)

INCLUDE = -I $(TOP)/include -I ../Common

SAMPLE_LIB = $(OUTPUT)/libsampleCommon.a

SOAMAPI_LIB = $(OUTPUT)/libsoamapi.a

ARCH_BUILD = linux-x86_64
LIBS = -L $(OUTPUT) -L $(TOP)/$(ARCH_BUILD)/lib64 \
   -lsampleCommon -lsoambase -lsoamapi 

CFLAGS_RELEASE = -DGCC34 -DLINUX
CFLAGS_DEBUG = -g -DGCC34 -DLINUX
CFLAGS_=$(CFLAGS_DEBUG)

OUTF = -o $@
OEXT =o

SOAMAPI_OBJS = $(OUTPUT)/SoamSrc.$(OEXT)

## sources and targets
EXEC =  AsyncClient
TARGET = $(OUTPUT)/$(EXEC)

OBJS = $(EXEC).$(OEXT) 

SRCS = $(OBJS:.${OEXT}=.cpp)

.SUFFIXES: .cpp .$(OEXT)

## building process

build all: ${TARGET} 

$(TARGET): $(SAMPLE_LIB) $(SOAMAPI_LIB) $(OBJS) 
	$(CC) $(OUTF) ${OBJS} $(LIBS)
	
%.$(OEXT):  %.cpp
	${CC} ${CFLAGS_$(MODE)} $(INCLUDE) -c $(OUTF) $<;

$(SAMPLE_LIB):
	cd ../Common; make ;

$(SOAMAPI_LIB): $(SOAMAPI_OBJS)
	ar cru $@ $(SOAMAPI_OBJS)

${OUTPUT}/%.$(OEXT):  $(TOP)/src/%.cpp
	$(CC) $(CFLAGS_$(MODE)) $(INCLUDE) -c $(OUTF) $<; 

clean:
	-rm -f $(OBJS) $(TARGET) $(SOAMAPI_LIB) $(SOAMAPI_OBJS)
