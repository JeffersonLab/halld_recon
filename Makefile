DIRS += external libraries programs

ifndef MAKECMDGOALS
MAKECMDGOALS += install
endif

include $(HALLD_HOME)/src/BMS/Makefile.dirs
