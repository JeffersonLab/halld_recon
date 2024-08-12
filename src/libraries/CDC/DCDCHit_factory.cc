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

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>

#include "DCDCHit_factory.h"

static double DIGI_THRESHOLD = -1.0e8;

//------------------
// Init
//------------------
void DCDCHit_factory::Init()
{
  auto app = GetApplication();
  USE_CDC=true;
  app->SetDefaultParameter("CDC:ENABLE",USE_CDC);
  if (USE_CDC==false) return; // RESOURCE_UNAVAILABLE;
  
  LowTCut = -10000.;
  HighTCut = 10000.;

  Disable_CDC_TimingCuts = 0; // this can be changed by a parameter in brun()
  // Note: That has to be done in brun() because the parameters are read from ccdb at every call to brun()

  app->SetDefaultParameter("CDCHit:Disable_TimingCuts", Disable_CDC_TimingCuts,
                              "Disable CDC timing Cuts if this variable is not zero!");
  

  app->SetDefaultParameter("CDC:DIGI_THRESHOLD",DIGI_THRESHOLD,
			      "Do not convert CDC digitized hits into DCDCHit objects"
			      " that would have q less than this");
  
  RemoveCorrelationHits = 1;
  
  app->SetDefaultParameter("CDCHit:RemoveCorrelationHits", RemoveCorrelationHits,
			      "Remove hits correlated in time with saturation hits!");
  
  CorrelationHitsCut = 1.5;
  app->SetDefaultParameter("CDCHit:CorrelationHitsCut", CorrelationHitsCut,
			      "Cut in units of 8ns bins to remove correlated hits with Saturation hits!");
  
  CorrelatedHitPeak = 3.5;
  app->SetDefaultParameter("CDCHit:CorrelatedHitPeak", CorrelatedHitPeak,
                              "Location of peak time around which we cut correlated times in units of 8ns bins");

  // Setting this flag makes it so that JANA does not delete the objects in _data.
  // This factory will manage this memory.
  SetFactoryFlag(NOT_OBJECT_OWNER);  // TODO: Make sure we don't need PERSISTENT as well
}

//------------------
// BeginRun
//------------------
void DCDCHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  if (USE_CDC==false) return; // RESOURCE_UNAVAILABLE;

  /// Read in calibration constants
  auto runnumber = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto jcalib = app->GetService<JCalibrationManager>()->GetJCalibration(runnumber);

  vector<double> cdc_timing_cuts;
  
  if (jcalib->Get("/CDC/timing_cut", cdc_timing_cuts)){
    LowTCut = -60.;
    HighTCut = 900.;
    jout << "Error loading /CDC/timing_cut ! set defaul values -60. and 900." << jendl;
  } else {
    LowTCut = cdc_timing_cuts[0];
    HighTCut = cdc_timing_cuts[1];
    //jout<<"CDC Timing Cuts: "<<LowTCut<<" ... "<<HighTCut<<endl;
  }
  
  app->SetDefaultParameter("CDCHit:LowTCut", LowTCut,"Minimum acceptable CDC hit time (ns)");
  app->SetDefaultParameter("CDCHit:HighTCut", HighTCut, "Maximum acceptable CDC hit time (ns)");
  
  //jout<<LowTCut<<" / "<<HighTCut<<endl;

  if (Disable_CDC_TimingCuts) {
    
    LowTCut = -10000.;
    HighTCut = 10000.;
    jout << "Disable CDC Hit Timing Cuts!" << jendl;
    
  }
  
  event->Get(ttab);
}

//------------------
// Process
//------------------
void DCDCHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{ 
  if (USE_CDC==false) return; // RESOURCE_UNAVAILABLE; // TODO: Verify
 
  // Clear _data vector
  mData.clear();

  vector<const DCDCHit*> hits;
  event->Get(hits, "Calib");
  
  
  vector<cdchit_info_t> hit_info_vec;
  
  // loop over hits and find roc/slot/con numbers
  for (unsigned int k=0 ;k<hits.size(); k++){
    const DCDCHit *hit = hits[k];
    vector <const Df125CDCPulse*> pulse;
    hit->Get(pulse);
    
    cdchit_info_t hit_info;
    if(pulse.size()==0) {
      // for hits without lower-level hit info, e.g. HDDM data, we have to use the translation table
      // to figure out which DAQ channels his hit corresponds to
      try {
	DTranslationTable::DChannelInfo channel_info;
	channel_info.det_sys = DTranslationTable::CDC;
	channel_info.cdc.ring = hit->ring;
	channel_info.cdc.straw = hit->straw;
	DTranslationTable::csc_t daq_index = ttab[0]->GetDAQIndex(channel_info);
	
	hit_info.rocid = daq_index.rocid;
	hit_info.slot = daq_index.slot;
	hit_info.connector = daq_index.channel / 24;
      } catch(...) { 
	cout << "Cannot find Translation Table data for hit on ring " << hit->ring
	     << " straw " << hit->straw << ", skipping this info ..." << endl;
	continue;
      }
    } else {
      hit_info.rocid = pulse[0]->rocid;
      hit_info.slot = pulse[0]->slot;
      hit_info.connector = pulse[0]->channel / 24;
    }
    
    hit_info.time = hit->t;
    hit_info.max = 0;
    
    if (hit->QF > 1) {
      hit_info.max = 1;
    }
    
    hit_info_vec.push_back(hit_info);
  }
  
  
  vector<bool> Mark4Removal(hit_info_vec.size(), false);
  
  if (RemoveCorrelationHits && (hit_info_vec.size()>0) ) {
    
    for (unsigned int k=0 ;k<hit_info_vec.size()-1; k++){
      
      if (hit_info_vec[k].max){
	
	for (unsigned int n=k+1 ;n<hit_info_vec.size(); n++){
	  
	  //if (n==k)
	  //  continue;
	  
	  //if ((RocID[k] == RocID[n]) && (Slot[k] == Slot[n]) && (Connector[k] == Connector[n]) ){
	  if(hit_info_vec[k] == hit_info_vec[n]) {
	    double dt = (hit_info_vec[k].time - hit_info_vec[n].time)/8.; // units of samples (8ns)
	    if ( std::fabs(dt+CorrelatedHitPeak)<CorrelationHitsCut) {
	      Mark4Removal[n] = true;
	      //cout<<"remove "<<hits[n]->ring<<" "<<hits[n]->straw<<endl;
	    }
            
	  }
          
	}
      }
    }
    
  }

  
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

    // removed hits correclated with Saturation hit on same connector/reamp/HV-board
    // this vector should not be filled if we are running over HDDM data
    if (Mark4Removal.size() > 0 && Mark4Removal[k]){
      continue;
    }
    
    Insert( const_cast<DCDCHit*>(hit) );
    
  }
}

//------------------
// EndRun
//------------------
void DCDCHit_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DCDCHit_factory::Finish()
{
}
