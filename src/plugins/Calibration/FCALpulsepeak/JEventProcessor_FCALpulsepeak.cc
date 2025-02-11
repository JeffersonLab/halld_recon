// $Id$
//
//    File: JEventProcessor_FCALpulsepeak.cc
// Created: Tue Sep 27 11:18:28 EDT 2016
// Creator: asubedi (on Linux stanley.physics.indiana.edu 2.6.32-573.18.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_FCALpulsepeak.h"


// Routine used to create our JEventProcessor
#include <stdint.h>
#include <vector>
#include <iostream>
#include "FCAL/DFCALHit.h"
#include "FCAL/DFCALDigiHit.h"
#include "FCAL/DFCALGeometry.h"
#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALShower.h"
#include "DAQ/Df250PulseIntegral.h"
#include "DAQ/Df250PulsePedestal.h"
#include "DAQ/Df250PulseData.h"
#include "units.h"
#include "DLorentzVector.h"
#include "DVector3.h"
#include "HDGEOMETRY/DGeometry.h"
#include "DANA/DEvent.h"
#include "TRIGGER/DTrigger.h"

#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <TDirectory.h>
#include <TH1I.h>
#include <TH2F.h>
#include "HistogramTools.h"


const int nChan = 2800;

// Define Histograms
static TH1I* pulsepeak[nChan];


extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_FCALpulsepeak());
}
} // "C"


//------------------
// JEventProcessor_FCALpulsepeak (Constructor)
//------------------
JEventProcessor_FCALpulsepeak::JEventProcessor_FCALpulsepeak()
{
	SetTypeName("JEventProcessor_FCALpulsepeak");
}

//------------------
// ~JEventProcessor_FCALpulsepeak (Destructor)
//------------------
JEventProcessor_FCALpulsepeak::~JEventProcessor_FCALpulsepeak()
{

}

//------------------
// Init
//------------------
void JEventProcessor_FCALpulsepeak::Init()
{

	//This is called once at program startup. 
TDirectory *main = gDirectory;
  gDirectory->mkdir("FCAL_pulsepeak")->cd();
  
  
  for (int i = 0; i < nChan; ++i) {
    pulsepeak[i] = new TH1I(Form("peak_%i",i),Form("Pulsepeak for Channel %i",i),500,10,-10);
  }

  main->cd();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_FCALpulsepeak::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes
}
//------------------
// Process
//------------------
void JEventProcessor_FCALpulsepeak::Process(const std::shared_ptr<const JEvent>& event)
{

    // select events with physics events, i.e., not LED and other front panel triggers
    const DTrigger* locTrigger = NULL; 
    event->GetSingle(locTrigger); 
    if(locTrigger->Get_L1FrontPanelTriggerBits() != 0) 
      return;

    vector< const DFCALDigiHit*  > digiHits;
    event->Get( digiHits );
    vector< const DFCALGeometry* > geomVec;
    event->Get( geomVec );
    
    const DFCALGeometry& fcalGeom = *(geomVec[0]);

    auto lockService = GetLockService(event);
    lockService->RootFillLock(this);

    for( vector< const DFCALDigiHit* >::const_iterator dHitItr = digiHits.begin();
         dHitItr != digiHits.end(); ++dHitItr ){
        const DFCALDigiHit& dHit = (**dHitItr);
        int m_chan = fcalGeom.channel( dHit.row, dHit.column );
        //uint32_t m_time = dHit.pulse_time;
        uint32_t m_peak = dHit.pulse_peak;
        uint32_t m_pedestal = dHit.pedestal;
        //uint32_t m_integral = dHit.pulse_integral;
        
        if (m_peak > 0 && m_pedestal > 95){
            
            pulsepeak[m_chan]->Fill(m_peak); 
            
        }
    }

    lockService->RootFillUnLock(this);
    


/*
	  vector< const DFCALGeometry* > geomVec;
  vector< const DFCALDigiHit*  > digiHits;
  vector< const DFCALHit*      > hits;
    event->Get( geomVec );
  event->Get( digiHits );
  event->Get( hits );
  
  
  const DFCALGeometry& fcalGeom = *(geomVec[0]);
 


  for( vector< const DFCALDigiHit* >::const_iterator dHitItr = digiHits.begin();
       dHitItr != digiHits.end(); ++dHitItr ){
  
    // fetch lower level FADC data
    const Df250PulseData*     pulseDat = NULL;

    const DFCALDigiHit& dHit = (**dHitItr);
    dHit.GetSingle( pulseDat );
    pd_cache[&dHit] = pulseDat;
  }
  
  for( vector< const DFCALDigiHit* >::const_iterator dHitItr = digiHits.begin();
       dHitItr != digiHits.end(); ++dHitItr ){

    const DFCALDigiHit& dHit = (**dHitItr);
   
    const Df250PulseData* pulseDat = pd_cache[&dHit];
    
    
    if( pulseDat){
      
      // m_x = dHit.column  - 29;
      // m_y = dHit.row  - 29;
       m_chan = fcalGeom.channel( dHit.row, dHit.column );
    //cout << "pedestal: " << pulseDat->pedestal << " peak: " << pulseDat->pulse_peak << endl;
    
       m_peak = pulseDat->pulse_peak;
   
       japp->RootFillLock(this);
       if (m_peak > 0){
       // cout << "Channel: " << m_chan << " peak: " << m_peak << endl;
      pulsepeak[m_chan]->Fill(m_peak); 
   // Fill the only important histogram
                //Fill2DHistogram("FCALpulsepeak", "", "FCAL Pulse Peak Vs. Channel Number",
                        //m_chan, m_peak,
                        //"Pulse Peak ;Channel Number; Pulse Peak",
                        //2800,-0.5,2799.5,4000,100, 4100); // Channels are numbered from zero...AGAINST CONVENTION    
     }
japp->RootFillUnLock(this);
 

	}
     }
*/  
  
  





}

//------------------
// EndRun
//------------------
void JEventProcessor_FCALpulsepeak::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_FCALpulsepeak::Finish()
{
	// Called before program exit after event processing is finished.
}

