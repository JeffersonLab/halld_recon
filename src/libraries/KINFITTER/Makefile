# Makefile to compile stand-alone shared KINFITTER library

CXX = g++
ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)
CXXFLAGS = -fPIC -Wall -O2 -std=c++11 -D_KINFITTER_STANDALONE_ -I../include -I../../external $(ROOTCFLAGS)
LDFLAGS = -shared $(ROOTLIBS)
SRCS = $(wildcard *.cc)
OBJDIR = lib
OBJS = $(patsubst %.cc,$(OBJDIR)/%.o,$(SRCS))
LIB = libKINFITTER.so

all: $(OBJDIR) $(OBJDIR)/$(LIB)

# ensure lib directory exists
$(OBJDIR):
	mkdir -p $(OBJDIR)

# link shared library into lib/
$(OBJDIR)/$(LIB): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

# compile source files into lib/*.o
$(OBJDIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

.PHONY: all clean
