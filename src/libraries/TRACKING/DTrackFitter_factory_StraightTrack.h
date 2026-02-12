// $Id$
//
//    File: DTrackFitter_factory_StraightTrack.h
// Created: Tue Mar 12 16:49:20 EDT 2019
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DTrackFitter_factory_StraightTrack_
#define _DTrackFitter_factory_StraightTrack_

#include <JANA/JFactoryT.h>
#include <TRACKING/DTrackFitterStraightTrack.h>

class DTrackFitter_factory_StraightTrack:public JFactoryT<DTrackFitter>{
 public:
  DTrackFitter_factory_StraightTrack(){
  	SetTag("StraightTrack");
  };
  ~DTrackFitter_factory_StraightTrack() override = default;

  DTrackFitter *fitter=nullptr;
  
  //------------------
  // BeginRun
  //------------------
  void BeginRun(const std::shared_ptr<const JEvent>& event) override
  {
    // (See DTAGHGeometry_factory.h)
    SetFactoryFlag(NOT_OBJECT_OWNER);
    ClearFactoryFlag(WRITE_TO_OUTPUT);
    
    delete fitter;
    fitter = new DTrackFitterStraightTrack(event);
  }
  
  //------------------
  // Process
  //------------------
  void Process(const std::shared_ptr<const JEvent>& event) override
  {
    // Reuse existing DTrackFitterKalmanSIMD object.
    if( fitter ) Insert( fitter );
  }
  
  //------------------
  // EndRun
  //------------------
  void EndRun() override
  {
    delete fitter;
    fitter = nullptr;
  }
};

#endif // _DTrackFitter_factory_StraightTrack_

