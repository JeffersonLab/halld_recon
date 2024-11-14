// $Id$
//
//    File: DCDCHit_factory.cc
// Created: Tue Aug  6 11:29:56 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//


#include <iostream>
#include <iomanip>
#include <cmath>
#include <CDC/DCDCHit.h>

#include "DCDCHit_factory.h"

static double DIGI_THRESHOLD = -1.0e8;

//------------------
// init
//------------------
jerror_t DCDCHit_factory::init(void)
{

  USE_CDC=true; 
  gPARMS->SetDefaultParameter("CDC:ENABLE",USE_CDC);
  if (USE_CDC==false) return RESOURCE_UNAVAILABLE;
  
  LowTCut = -10000.;
  HighTCut = 10000.;

  Disable_CDC_TimingCuts = 0; // this can be changed by a parameter in brun()
  // Note: That has to be done in brun() because the parameters are read from ccdb at every call to brun()
  gPARMS->SetDefaultParameter("CDCHit:Disable_TimingCuts", Disable_CDC_TimingCuts,
                              "Disable CDC timing Cuts if this variable is not zero!");
  

  gPARMS->SetDefaultParameter("CDC:DIGI_THRESHOLD",DIGI_THRESHOLD,
			      "Do not convert CDC digitized hits into DCDCHit objects"
			      " that would have q less than this");
  
  // Setting this flag makes it so that JANA does not delete the objects in _data.
  // This factory will manage this memory.
  SetFactoryFlag(NOT_OBJECT_OWNER);
  
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DCDCHit_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
  if (USE_CDC==false) return RESOURCE_UNAVAILABLE;

  /// Read in calibration constants
  
  vector<double> cdc_timing_cuts;
  
  if (eventLoop->GetCalib("/CDC/timing_cut", cdc_timing_cuts)){
    LowTCut = -60.;
    HighTCut = 900.;
    jout << "Error loading /CDC/timing_cut ! set defaul values -60. and 900." << endl;
  } else {
    LowTCut = cdc_timing_cuts[0];
    HighTCut = cdc_timing_cuts[1];
    //jout<<"CDC Timing Cuts: "<<LowTCut<<" ... "<<HighTCut<<endl;
  }
  
  gPARMS->SetDefaultParameter("CDCHit:LowTCut", LowTCut,"Minimum acceptable CDC hit time (ns)");
  gPARMS->SetDefaultParameter("CDCHit:HighTCut", HighTCut, "Maximum acceptable CDC hit time (ns)");
  
  //jout<<LowTCut<<" / "<<HighTCut<<endl;

  if (Disable_CDC_TimingCuts) {
    
    LowTCut = -10000.;
    HighTCut = 10000.;
    jout << "Disable CDC Hit Timing Cuts!" << endl;
    
  }
  
  eventLoop->Get(ttab);
  
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DCDCHit_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{ 
  if (USE_CDC==false) return RESOURCE_UNAVAILABLE;
 
  // Clear _data vector
  _data.clear();  
  
  vector<const DCDCHit*> hits;
  loop->Get(hits, "Calib");
  
  
  for (int k=0 ;k<(int) hits.size(); k++){
    const DCDCHit *hit = hits[k];

    // remove hits with small charge: not really used
    if (hit->q < DIGI_THRESHOLD) {
      continue;
    }

    // remove hits with amplitudes 0 or less
    if ( hit->amp <= 0 ) { 
      continue;
     }

    // remove hits with failed timing alorithm
    if ( (hit->QF & 0x1) != 0 ) { 
        continue;
    }

    // remove hits ouside of the timing cut
    if ( (hit->t < LowTCut) || (hit->t > HighTCut) ){
      continue;
    }
    
    _data.push_back( const_cast<DCDCHit*>(hit) );
    
  }
  
  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DCDCHit_factory::erun(void)
{
    return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DCDCHit_factory::fini(void)
{
    return NOERROR;
}
