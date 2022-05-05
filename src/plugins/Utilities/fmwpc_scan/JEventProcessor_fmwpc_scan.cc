// $Id$
//
//    File: JEventProcessor_fmwpc_scan.cc
// Created: Wed Nov 24 19:47:15 EST 2021
// Creator: njarvis (on Linux ifarm1802.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_fmwpc_scan.h"
using namespace std;
using namespace jana;

#include <stdint.h>
#include <vector>
#include <TTree.h>
#include <TBranch.h>

#include "FMWPC/DFMWPCDigiHit.h"
#include "DAQ/Df125WindowRawData.h"     
#include "DAQ/Df125CDCPulse.h"

#include "TRIGGER/DTrigger.h"


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_fmwpc_scan());
}
} // "C"


//------------------
// JEventProcessor_fmwpc_scan (Constructor)
//------------------
JEventProcessor_fmwpc_scan::JEventProcessor_fmwpc_scan()
{

}

//------------------
// ~JEventProcessor_fmwpc_scan (Destructor)
//------------------
JEventProcessor_fmwpc_scan::~JEventProcessor_fmwpc_scan()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_fmwpc_scan::init(void)
{
	// This is called once at program startup. 

  const uint32_t NSAMPLES = 300;

  japp->RootWriteLock();

  t = new TTree("T","FWMPC events");
  
  uint32_t eventnum;
  t->Branch("eventnum",&eventnum,"eventnum/i");

  uint32_t rocid;
  t->Branch("rocid",&rocid,"rocid/i");

  uint32_t slot;
  t->Branch("slot",&slot,"slot/i");
  
  uint32_t channel;
  t->Branch("channel",&channel,"channel/i");

  uint32_t itrigger;
  t->Branch("itrigger",&itrigger,"itrigger/i");

  uint32_t word1;
  t->Branch("word1",&word1,"word1/i");

  uint32_t word2; 
  t->Branch("word2",&word2,"word2/i");

  uint32_t time;   
  t->Branch("time",&time,"time/i");    

  uint32_t amp;
  t->Branch("amp",&amp,"amp/i");    

  uint32_t pedestal;  
  t->Branch("pedestal",&pedestal,"pedestal/i");    

  uint32_t integral;
  t->Branch("integral",&integral,"integral/i");    

  uint32_t q;
  t->Branch("q",&q,"q/i");    

  uint32_t overflows; 
  t->Branch("overflows",&overflows,"overflows/i");    

  bool emulated;//  
  t->Branch("emulated",&emulated,"emulated/O");    

  uint32_t ns;
  t->Branch("nsamples",&ns,"nsamples/i");

  uint16_t adc[NSAMPLES];
  t->Branch("adc",&adc,Form("adc[%i]/s",NSAMPLES));         

  japp->RootUnLock();


  return NOERROR;

}

//------------------
// brun
//------------------
jerror_t JEventProcessor_fmwpc_scan::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_fmwpc_scan::evnt(JEventLoop *loop, uint64_t eventnumber)
{

  /*

  // Only look at physics triggers

  const DTrigger* locTrigger = NULL; 
  loop->GetSingle(locTrigger); 
  if(locTrigger->Get_L1FrontPanelTriggerBits() != 0)
    return NOERROR;
  if (!locTrigger->Get_IsPhysicsEvent()){ // do not look at PS triggers
    return NOERROR;
  }

  */


  vector<const DFMWPCDigiHit*> mwhitsvector;
  loop->Get(mwhitsvector);

  uint nmp = (uint)mwhitsvector.size();

  if (!nmp) return NOERROR;


  const uint NSAMPLES=300;

  for (uint32_t i=0; i<nmp; i++) {

      const DFMWPCDigiHit *mwhit = mwhitsvector[i];

      const Df125CDCPulse *cp = NULL;
      mwhit->GetSingle(cp);

      const Df125WindowRawData *wrd = NULL;
      cp->GetSingle(wrd);

      uint32_t eventnum = eventnumber&0xFFFFFFFF;
      uint32_t rocid = cp->rocid;
      uint32_t slot = cp->slot;
      uint32_t channel = cp->channel;
      uint32_t itrigger = cp->itrigger;

      uint32_t word1 = cp->word1;
      uint32_t word2 = cp->word2;
      uint32_t time = cp->le_time;
      uint32_t pedestal = cp->pedestal;
      uint32_t integral = cp->integral;
      uint32_t q = cp->time_quality_bit;
      uint32_t overflows = cp->overflow_count;
      uint32_t amp = cp->first_max_amp;

      bool emulated = cp->emulated;

      uint16_t adc[NSAMPLES]= {0};

      uint32_t ns = (uint32_t)wrd->samples.size();

      for (uint j=0; j<ns; j++) {
          adc[j] = wrd->samples[j];
      }


      japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!

      t->SetBranchAddress("eventnum",&eventnum);
      t->SetBranchAddress("rocid",&rocid);
      t->SetBranchAddress("slot",&slot);
      t->SetBranchAddress("channel",&channel);
      t->SetBranchAddress("itrigger",&itrigger); 
      t->SetBranchAddress("word1",&word1);
      t->SetBranchAddress("word2",&word2);

      t->SetBranchAddress("pedestal",&pedestal);
      t->SetBranchAddress("integral",&integral);
      t->SetBranchAddress("amp",&amp);
      t->SetBranchAddress("time",&time);
      t->SetBranchAddress("q",&q);
      t->SetBranchAddress("overflows",&overflows);
      t->SetBranchAddress("emulated",&emulated);

      t->SetBranchAddress("nsamples",&ns);
      t->SetBranchAddress("adc",&adc);
      
      t->Fill();
      japp->RootUnLock();
  }



  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_fmwpc_scan::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_fmwpc_scan::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}

