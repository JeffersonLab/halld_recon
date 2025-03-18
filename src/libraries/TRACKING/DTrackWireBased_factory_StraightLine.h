// $Id$
//
//    File: DTrackWireBased_factory_StraightLine.h
// Created: Wed Mar 13 10:00:25 EDT 2019
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DTrackWireBased_factory_StraightLine_
#define _DTrackWireBased_factory_StraightLine_

#include <JANA/JFactoryT.h>
#include "DTrackWireBased.h"
#include <TRACKING/DTrackFitter.h>
#include <TRACKING/DTrackFinder.h>
#include <TRACKING/DTrackCandidate.h>
#include <PID/DParticleID.h>

class DTrackWireBased_factory_StraightLine:public JFactoryT<DTrackWireBased>{
 public:
  DTrackWireBased_factory_StraightLine(){
    SetTag("StraightLine");
  };
  ~DTrackWireBased_factory_StraightLine(){};

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  double CDC_MATCH_CUT, FDC_MATCH_CUT;
  
  const DParticleID* dPIDAlgorithm;
  DTrackFitter *fitter;
  DTrackFinder *finder;
  
};

#endif // _DTrackWireBased_factory_StraightLine_

