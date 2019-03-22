// $Id$
//
//    File: DTrackFitter_factory_StraightTrack.h
// Created: Tue Mar 12 16:49:20 EDT 2019
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DTrackFitter_factory_StraightTrack_
#define _DTrackFitter_factory_StraightTrack_

#include <JANA/JFactory.h>
#include <TRACKING/DTrackFitterStraightTrack.h>

class DTrackFitter_factory_StraightTrack:public jana::JFactory<DTrackFitter>{
 public:
  DTrackFitter_factory_StraightTrack(){};
  ~DTrackFitter_factory_StraightTrack(){};
  const char* Tag(void){return "StraightTrack";}
  
  DTrackFitter *fitter=NULL;
  
  //------------------
  // brun
  //------------------
  jerror_t brun(JEventLoop *loop, int32_t runnumber)
  {
    // (See DTAGHGeometry_factory.h)
    SetFactoryFlag(NOT_OBJECT_OWNER);
    ClearFactoryFlag(WRITE_TO_OUTPUT);
    
    if( fitter ) delete fitter;
    
    fitter = new DTrackFitterStraightTrack(loop);

    return NOERROR;
  }
  
  //------------------
  // evnt
  //------------------
  jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
  {
    // Reuse existing DTrackFitterKalmanSIMD object.
    if( fitter ) _data.push_back( fitter );
    
    return NOERROR;
  }
  
  //------------------
  // erun
  //------------------
  jerror_t erun(void)
  {
    if( fitter ) delete fitter;	
    fitter = NULL;
    
    return NOERROR;
  }
};

#endif // _DTrackFitter_factory_StraightTrack_

