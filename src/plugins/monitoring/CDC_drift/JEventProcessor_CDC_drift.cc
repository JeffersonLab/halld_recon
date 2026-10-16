// $Id$
//
//    File: JEventProcessor_CDC_drift.cc
// Created: Wed Oct 22 2014
// Creator: Naomi Jarvis


#include <stdint.h>
#include <vector>

#include <TMath.h>


#include "JEventProcessor_CDC_drift.h"
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>


using namespace std;


#include "CDC/DCDCHit.h"

#include "TRACKING/DTrackTimeBased.h"
#include <PID/DChargedTrackHypothesis.h>
#include <PID/DChargedTrack.h>
#include <TRACKING/DTrackFitter.h>
#include "PID/DVertex.h"


#include "TRIGGER/DTrigger.h"

#include <TDirectory.h>

#include <TH1.h>
#include <TF1.h>


//----------------------------------------------------------------------------------


// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_CDC_drift());
  }
}


//----------------------------------------------------------------------------------


JEventProcessor_CDC_drift::JEventProcessor_CDC_drift() {
	SetTypeName("JEventProcessor_CDC_drift");
}


//----------------------------------------------------------------------------------


JEventProcessor_CDC_drift::~JEventProcessor_CDC_drift() {
}


//----------------------------------------------------------------------------------

void JEventProcessor_CDC_drift::Init() {

  auto app = GetApplication();
  lockService = app->GetService<JLockService>();

  /*
  // max values for histogram scales, modified fa250-format readout
  const Int_t RTMAX = 12000; //max for raw time, less than full field width
  const Char_t rtunits[8] = "0.125ns";  //raw time is in units of sample/64 = ns/8
  */


  // raw quantities for read out (125 format) are
  //   time                    field max 2047   scaled x 1, units 0.8ns
  //   time qf                 field max 1 
  //   overflow count          field max 7
  //   pedestal                field max 255    scaled x 1/4 initially
  //   max amplitude 9 bits,   field max 511    scaled x 1/8
  //   integral                field max 16383  scaled x 1/14


  // max values for histogram scales, fa125-format readout


  const Int_t TMAX = 2000;  //max for time


  // create root folder for cdc and cd to it, store main dir
  TDirectory *main = gDirectory;
  gDirectory->mkdir("CDC_drift")->cd();

  cdc_time = new TH1F("cdc_time","CDC time (units of ns); time (ns)",TMAX,-500,TMAX-500);
  main->cd();
}


//----------------------------------------------------------------------------------


void JEventProcessor_CDC_drift::BeginRun(const std::shared_ptr<const JEvent>& event) {
  // This is called whenever the run number changes
}


//----------------------------------------------------------------------------------


void JEventProcessor_CDC_drift::Process(const std::shared_ptr<const JEvent>& event) {
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop-Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.

  // cosmics, estimate 15 mins ~ 4.4e5 events ~ 4.4e5*82/372 ~ 1e5 useful hits




  const DTrigger* locTrigger = NULL; 
  event->GetSingle(locTrigger); 
  if (locTrigger->Get_L1FrontPanelTriggerBits()) return;
  if (!locTrigger->Get_IsPhysicsEvent()) return;  // do not look at PS triggers

	
  const DVertex* locVertex  = NULL;
  event->GetSingle(locVertex);
  double vertexz = locVertex->dSpacetimeVertex.Z();
  if ((vertexz < 52.0) || (vertexz > 78.0)) return;


  //array to make straw number n; add extra 0 at front to use offset[1] for ring 1
  int straw_offset[29] = {0,0,42,84,138,192,258,324,404,484,577,670,776,882,1005,1128,1263,1398,1544,1690,1848,2006,2176,2346,2528,2710,2907,3104,3313};

  const int nstraws = 77;  //size of strawlist - list of n of straws to include in fit

  const int strawlist[] = {176, 237, 496, 497, 775, 776, 777, 782, 879, 881, 882, 895, 900, 1021, 1026, 1047, 1052, 1056, 1057, 1130, 1241, 1252, 1266, 1318, 1340, 1376, 1567, 1568, 1679, 1682, 1701, 1849, 1853, 1864, 1918, 1998, 2088, 2242, 2244, 2248, 2255, 2256, 2430, 2445, 2556, 2585, 2748, 2767, 2770, 2772, 2774, 2782, 2788, 2789, 2793, 2796, 2943, 2951, 2952, 2962, 2963, 2965, 2969, 2973, 2985, 3159, 3160, 3176, 3177, 3184, 3214, 3361, 3363, 3365, 3369, 3428, 3429};


  
  vector<const DChargedTrack*> ctracks;
  event->Get(ctracks);

  for (uint32_t i=0; i<(uint32_t)ctracks.size(); i++) {  

    // get the best hypo
    const DChargedTrackHypothesis *hyp=ctracks[i]->Get_BestFOM();    
    if (hyp == NULL) continue;
      
    const DTrackTimeBased *track = hyp->Get_TrackTimeBased();
    //    uint16_t ntrackhits_cdc = (uint16_t)track->measured_cdc_hits_on_track;

    if (!track) continue;
    
    vector<DTrackFitter::pull_t> pulls = track->pulls;

    for (uint32_t j=0; j<pulls.size(); j++) {

      if (pulls[j].cdc_hit == NULL) continue;

      const DCDCHit *hit = NULL;
      pulls[j].cdc_hit->GetSingle(hit);

      if (!hit) continue;
      
      double t = hit->t;
      int QF = hit->QF;  // 0 is good

      if (QF) continue;

      int ring = hit->ring;
      int straw = hit->straw;      
      int n = straw_offset[ring] + straw;

      bool fill_histo = false;

      for (int k=0; k<nstraws; k++) {
	if ( strawlist[k] == n) fill_histo = true;
        if (fill_histo) break;
	if (strawlist[k] > n) break;
      }

      if (!fill_histo) continue;

      lockService->RootFillLock(this); //ACQUIRE ROOT LOCK!!
      cdc_time->Fill(t);
      lockService->RootFillUnLock(this);
      
    } // pulls loop

  }  // tracks loop
}


//----------------------------------------------------------------------------------


void JEventProcessor_CDC_drift::EndRun() {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}


//----------------------------------------------------------------------------------


void JEventProcessor_CDC_drift::Finish() {
  // Called before program exit after event processing is finished.
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
