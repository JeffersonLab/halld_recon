// $Id$
//
//    File: DPSCPair_factory.h
// Created: Tue Mar 24 21:35:49 EDT 2015
// Creator: nsparks (on Linux cua2.jlab.org 2.6.32-431.5.1.el6.x86_64 x86_64)
//

#ifndef _DPSCPair_factory_
#define _DPSCPair_factory_

#include <JANA/JFactoryT.h>
#include "DPSCPair.h"

class DPSCPair_factory:public JFactoryT<DPSCPair>{
 public:
  DPSCPair_factory(){};
  ~DPSCPair_factory(){};

  double DELTA_T_PAIR_MAX;

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
};

#endif // _DPSCPair_factory_

