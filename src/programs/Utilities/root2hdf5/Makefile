hdf5dir = /nfs/direct/packages/hdf5/hdf5-1.10.5-linux-centos7-x86_64-shared
rootlibs = $(shell root-config --libs)
rootincs = $(shell root-config --incdir)
rootlibs += -lNetx

#include $(HALLD_HOME)/src/BMS/Makefile.bin

root2hdf5: root2hdf5.cc root2hdf5.h
	g++ -std=c++20 -g -o $@ -I$(rootincs) $^ $(rootlibs) -lhdf5

root2hdf5_1.10: root2hdf5.cc root2hdf5.h
	g++ -std=c++20 -g -o $@ -I$(rootincs) -I$(hdf5dir)/include $^ $(rootlibs) -L$(hdf5dir)/lib -lhdf5

t: t.C
	g++ -std=c++20 -g -o $@ -I$(rootincs) $^ $(rootlibs) -lhdf5

clean:
	rm -f root2hdf5 *.o

