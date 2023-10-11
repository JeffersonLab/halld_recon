// $Id$
//
//    File: DTrackFinder_factory.h
// Created: Mon Aug 18 11:00:33 EDT 2014
// Creator: staylor (on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _DTrackFinder_factory_
#define _DTrackFinder_factory_

#include <JANA/JFactoryT.h>
#include "DTrackFinder.h"

class DTrackFinder_factory:public JFactoryT<DTrackFinder>{
 public:
  DTrackFinder_factory() = default;
  ~DTrackFinder_factory() override = default;

 private:
  void Process(const std::shared_ptr<const JEvent>& event) override {
    
    SetFactoryFlag(PERSISTENT);
    ClearFactoryFlag(WRITE_TO_OUTPUT);
   
    Insert(new DTrackFinder(GetApplication()));
  }
};

#endif // _DTrackFinder_factory_

