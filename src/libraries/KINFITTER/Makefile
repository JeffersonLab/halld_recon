# Makefile for stand-alone version of KINFITTER

CXX = g++
ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)
CXXFLAGS = -fPIC -Wall -O2 -std=c++11 -D_KINFITTER_STANDALONE_ -I../include -I../../external $(ROOTCFLAGS)
LDFLAGS = -shared $(ROOTLIBS)
SRCS = $(wildcard *.cc)
OBJS = $(SRCS:.cc=.o)
LIB = libKINFITTER.so

all: $(LIB)

$(LIB): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIB)
