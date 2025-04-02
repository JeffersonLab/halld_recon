// $Id$
//
//    File: DECALShower_factory.h
// Created: Tue Mar 25 10:43:23 AM EDT 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

#ifndef _DECALShower_factory_
#define _DECALShower_factory_

#include <JANA/JFactoryT.h>
#include "DECALCluster.h"
#include "DECALShower.h"

class DECALShower_factory:public JFactoryT<DECALShower>{
public:
  DECALShower_factory(){}
  ~DECALShower_factory(){}

private:
  void Init() override;                       ///< Called once at program start.
  void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
  void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
  void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
  void Finish() override;                     ///< Called after last event of last event source has been processed.

  double mECALz;
  double E_VAR1,E_VAR2,E_VAR3,POS_RES1,POS_RES2;
};

#endif // _DECALShower_factory_

