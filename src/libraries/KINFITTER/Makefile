# Makefile to compile stand-alone shared KINFITTER library

CXX = g++
ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)
CXXFLAGS = -fPIC -Wall -O2 -std=c++11 -D_KINFITTER_STANDALONE_ -I../include -I../../external $(ROOTCFLAGS)
LDFLAGS = -shared $(ROOTLIBS)
SRCS = $(wildcard *.cc)
OBJECT_DIR = obj
OBJS = $(patsubst %.cc,$(OBJECT_DIR)/%.o,$(SRCS))
LIB = libKINFITTER.so
INSTALL_DIR = $(HALLD_RECON_HOME)/$(BMS_OSNAME)/lib

all: $(OBJECT_DIR) $(OBJECT_DIR)/$(LIB)

# ensure directory for object files exists
$(OBJECT_DIR):
	mkdir -p $(OBJECT_DIR)

# link shared library into OBJECT_DIR
$(OBJECT_DIR)/$(LIB): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)
	@echo "Installing $(LIB) to $(INSTALL_DIR)"
	@mkdir -p "$(INSTALL_DIR)"
	@cp "$@" "$(INSTALL_DIR)/"
	@rm -f "$@"

# compile source files into OBJECT_DIR/*.o
$(OBJECT_DIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECT_DIR)

.PHONY: all clean
