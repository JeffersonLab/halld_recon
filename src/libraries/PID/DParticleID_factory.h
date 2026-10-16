// $Id$
//
//    File: DParticleID_factory.h
// Created: Mon Feb 28 13:47:49 EST 2011
// Creator: staylor (on Linux ifarml1 2.6.18-128.el5 x86_64)
//

#ifndef _DParticleID_factory_
#define _DParticleID_factory_

#include "DParticleID.h"

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>

class DParticleID_factory:public JFactoryT<DParticleID>{
 public:
  DParticleID_factory(){};
  ~DParticleID_factory(){};
  
 private:
  void Process(const std::shared_ptr<const JEvent>& event) override {
    // This is a trivial class that simply implements a default
    // factory. It is here so that the default can be changed 
    // easily by simply changing the tag here or on the command 
    // line.
    vector<const DParticleID*> pid_algorithms;
    event->Get(pid_algorithms,"PID1");
    for(unsigned int i=0; i< pid_algorithms.size(); i++){
      Insert(const_cast<DParticleID*>(pid_algorithms[i]));
    }
    SetFactoryFlag(NOT_OBJECT_OWNER);
  }
};

#endif // _DParticleID_factory_

