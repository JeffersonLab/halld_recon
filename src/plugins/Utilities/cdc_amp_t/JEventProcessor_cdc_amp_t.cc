// $Id$
//
//    File: JEventProcessor_cdc_amp_t.cc
// Created: Wed Aug 14 12:10:19 PM EDT 2024
// Creator: njarvis (on Linux gluon01 5.14.0-427.22.1.el9_4.x86_64 x86_64)
//

#include "JEventProcessor_cdc_amp_t.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>

#include "CDC/DCDCHit.h"
#include "CDC/DCDCDigiHit.h"

#include "DAQ/Df125CDCPulse.h"
#include "DAQ/Df125Config.h"

#include "TRIGGER/DTrigger.h"

#include <TRACKING/DTrackTimeBased.h>
#include <PID/DChargedTrackHypothesis.h>
#include <PID/DChargedTrack.h>
#include <TRACKING/DTrackFitter.h>
#include "CDC/DCDCTrackHit.h"




extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_cdc_amp_t());
}
} // "C"


//------------------
// JEventProcessor_cdc_amp_t (Constructor)
//------------------
JEventProcessor_cdc_amp_t::JEventProcessor_cdc_amp_t()
{

}

//------------------
// ~JEventProcessor_cdc_amp_t (Destructor)
//------------------
JEventProcessor_cdc_amp_t::~JEventProcessor_cdc_amp_t()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_cdc_amp_t::init(void)
{
	// This is called once at program startup. 

    amp_t = new TH2I("amp_t","Digihit pulse height - pedestal vs sample number;sample number;pulse height-pedestal", 200,0,200,205,0,4100);
    amp_tt = new TH2I("amp_tt","Digihit pulse height - pedestal vs sample number, hits on tracks;sample number; pulse height-pedestal", 200,0,200,205,0,4100);
  
    hitamp_t = new TH2I("hitamp_t","Hit amplitude vs time;time (ns);hit amplitude", 125,0,1000,205,0,4100);
    hitamp_tt = new TH2I("hitamp_tt","Tracked hit amplitude vs time;time (ns);hit amplitude", 125,0,1000,205,0,4100);

    return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_cdc_amp_t::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_cdc_amp_t::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// loop->Get(mydataclasses);
	//
	// japp->RootFillLock(this);
	//  ... fill historgrams or trees ...
	// japp->RootFillUnLock(this);


    // Only look at physics triggers
    
    const DTrigger* locTrigger = NULL; 
    loop->GetSingle(locTrigger); 
    if(locTrigger->Get_L1FrontPanelTriggerBits() != 0)
      return NOERROR;
    if (!locTrigger->Get_IsPhysicsEvent()){ // do not look at PS triggers
      return NOERROR;
    }

    // digihits/cdcpulses
    
    vector <const Df125CDCPulse*> cdcpulses;
    loop->Get(cdcpulses);
  
    if (cdcpulses.size()>0) {

      uint16_t ABIT=3;
      uint16_t PBIT=0;
      
      const Df125Config* config = NULL;
      cdcpulses[0]->GetSingle(config);
  
      if (config) ABIT=config->ABIT;
      if (config) PBIT=config->PBIT; 

      for (unsigned int i=0; i<(unsigned int)cdcpulses.size(); i++) {
	
        const Df125CDCPulse* cp = cdcpulses[i];
	uint32_t rought = (uint32_t)(cp->le_time/10);
        uint32_t net_amp = (cp->first_max_amp<<ABIT) - (cp->pedestal<<PBIT) ;

        japp->RootWriteLock();    
        amp_t->Fill((int)rought,(int)net_amp);
        japp->RootUnLock();    
      }
    }

    
    // hits
    
     
    vector <const DCDCHit*> cdchits;
    loop->Get(cdchits);
    uint32_t nh = (uint32_t)cdchits.size();
    
    for (uint32_t i=0; i<nh; i++) {
      
      const DCDCHit *hit = cdchits[i];

      japp->RootWriteLock();    
      hitamp_t->Fill(hit->t,(int)hit->amp);

      japp->RootUnLock();          
    }
    

    vector<const DChargedTrack*> ctracks;
    loop->Get(ctracks);
    
    for (uint32_t i=0; i<(uint32_t)ctracks.size(); i++) {  
      
      // get the best hypo
      const DChargedTrackHypothesis *hyp=ctracks[i]->Get_BestFOM();    
      if (hyp == NULL) continue;
      
      const DTrackTimeBased *track = hyp->Get_TrackTimeBased();
      
      vector<DTrackFitter::pull_t> pulls = track->pulls;

      for (uint32_t j=0; j<(uint32_t)pulls.size(); j++) {

        if (pulls[j].cdc_hit == NULL) continue;

        const DCDCHit *hit = NULL;
        pulls[j].cdc_hit->GetSingle(hit);

        // fill histo for hits on tracks 
        japp->RootWriteLock();    
        hitamp_tt->Fill(hit->t,(int)hit->amp);
        japp->RootUnLock();


        vector <const Df125CDCPulse*> cdcpulses;

	hit->Get(cdcpulses);

	if (cdcpulses.size() > 0) {

          const Df125CDCPulse* cp = cdcpulses[i];

	  uint16_t ABIT=3;
          uint16_t PBIT=0;
      
          const Df125Config* config = NULL;
          cp->GetSingle(config);
  
          if (config) ABIT=config->ABIT;
          if (config) PBIT=config->PBIT; 

  	  uint32_t rought = (uint32_t)(cp->le_time/10);
          uint32_t net_amp = (cp->first_max_amp<<ABIT) - (cp->pedestal<<PBIT) ;

          japp->RootWriteLock();    
          amp_tt->Fill((int)rought,(int)net_amp);
          japp->RootUnLock();
	}

	
      }
    }	

    


  

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_cdc_amp_t::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_cdc_amp_t::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}

