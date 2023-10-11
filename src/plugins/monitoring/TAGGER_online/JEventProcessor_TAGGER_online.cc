// $Id$
//
//    File: JEventProcessor_TAGGER_online.cc
// Created: Thu Feb 18 07:45:18 EST 2016
// Creator: jrsteven (on Linux gluon110.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//

#include "JEventProcessor_TAGGER_online.h"


// Routine used to create our JEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_TAGGER_online());
}
} // "C"


//------------------
// JEventProcessor_TAGGER_online (Constructor)
//------------------
JEventProcessor_TAGGER_online::JEventProcessor_TAGGER_online()
{
	SetTypeName("JEventProcessor_TAGGER_online");
}

//------------------
// ~JEventProcessor_TAGGER_online (Destructor)
//------------------
JEventProcessor_TAGGER_online::~JEventProcessor_TAGGER_online()
{

}

//------------------
// Init
//------------------
void JEventProcessor_TAGGER_online::Init()
{
	auto app = GetApplication();
	lockService = app->GetService<JLockService>();

	gDirectory->Cd("/");
	new TDirectoryFile("TAGGER", "TAGGER");
	gDirectory->cd("TAGGER");

	dTAGMPulsePeak_Column = new TH2D("TAGMPulsePeak_Column", "TAGM pulse peak vs column", 102, 0., 102., 200, 0., 2000.);
	dTAGMIntegral_Column = new TH2D("TAGMIntegral_Column", "TAGM pulse integral vs column", 102, 0., 102., 100, 0., 4000.);
	dTaggerEnergy_DeltaTSC = new TH2D("TaggerEnergy_DeltaTSC", "Tagger Energy vs. #Delta t (TAG-SC); #Delta t (TAG-SC); Tagger Energy", 200, -100, 100, 240, 0., 12.); 

	gDirectory->cd("..");
}

//------------------
// BeginRun
//------------------
void JEventProcessor_TAGGER_online::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_TAGGER_online::Process(const std::shared_ptr<const JEvent>& event)
{
        vector<const DBeamPhoton*> locBeamPhotons;
	event->Get(locBeamPhotons);

	vector<const DSCHit*> locSCHits;
	event->Get(locSCHits);

	for(size_t loc_i = 0; loc_i < locBeamPhotons.size(); loc_i++) {
	  const DTAGMHit* locTAGMHit;	  
	  locBeamPhotons[loc_i]->GetSingle(locTAGMHit);
	  if(locTAGMHit != NULL) { 
		// FILL HISTOGRAMS
		// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
		lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK
		dTAGMPulsePeak_Column->Fill(locTAGMHit->column, locTAGMHit->pulse_peak);
		dTAGMIntegral_Column->Fill(locTAGMHit->column, locTAGMHit->integral);
		lockService->RootUnLock(); //RELEASE ROOT FILL LOCK
		
		// add threshold on TAGM hits
		if(locTAGMHit->integral < 500.) continue;
	  }

	  for(size_t loc_j = 0; loc_j < locSCHits.size(); loc_j++) {
	    Double_t locDeltaT = locBeamPhotons[loc_i]->time() - locSCHits[loc_j]->t;
		// FILL HISTOGRAMS
		// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
		lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK
	    dTaggerEnergy_DeltaTSC->Fill(locDeltaT, locBeamPhotons[loc_i]->momentum().Mag());
		lockService->RootUnLock(); //RELEASE ROOT FILL LOCK

	  }
	}
}

//------------------
// EndRun
//------------------
void JEventProcessor_TAGGER_online::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_TAGGER_online::Finish()
{
	// Called before program exit after event processing is finished.
}

