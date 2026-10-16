// $Id$
//
//    File: DEventProcessor_trackeff_missing.cc
// Created: Wed Feb 25 08:58:19 EST 2015
// Creator: pmatt (on Linux pmattdesktop.jlab.org 2.6.32-504.8.1.el6.x86_64 x86_64)
//

#include "DEventProcessor_trackeff_missing.h"

// Routine used to create our DEventProcessor

extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->Add(new DEventProcessor_trackeff_missing()); //register this plugin
		locApplication->Add(new DFactoryGenerator_trackeff_missing()); //register the factory generator
	}
} // "C"

//------------------
// Init
//------------------
void DEventProcessor_trackeff_missing::Init()
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:
	//
	// lockService->RootWriteLock();
	//  ... create historgrams or trees ...
	// lockService->RootUnLock();
	//
}

//------------------
// BeginRun
//------------------
void DEventProcessor_trackeff_missing::BeginRun(const std::shared_ptr<const JEvent>& t)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void DEventProcessor_trackeff_missing::Process(const std::shared_ptr<const JEvent> &locEvent)
{
	//Optional: Get the analysis results for all DReactions. 
		//Getting these objects triggers the analysis, if it wasn't performed already. 
		//These objects contain the DParticleCombo objects that survived the DAnalysisAction cuts that were added to the DReactions
	vector<const DAnalysisResults*> locAnalysisResultsVector;
	locEvent->Get(locAnalysisResultsVector);
}

//------------------
// EndRun
//------------------
void DEventProcessor_trackeff_missing::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void DEventProcessor_trackeff_missing::Finish()
{
	// Called before program exit after event processing is finished.
}
