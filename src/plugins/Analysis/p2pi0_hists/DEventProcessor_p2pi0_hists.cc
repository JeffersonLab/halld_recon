// $Id$
//
//    File: DEventProcessor_p2pi0_hists.cc
// Created: Tue May 19 07:56:16 EDT 2015
// Creator: jrsteven (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#include "DEventProcessor_p2pi0_hists.h"

// Routine used to create our DEventProcessor

extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->Add(new DEventProcessor_p2pi0_hists()); //register this plugin
		locApplication->Add(new DFactoryGenerator_p2pi0_hists()); //register the factory generator
	}
} // "C"

//------------------
// Init
//------------------
void DEventProcessor_p2pi0_hists::Init()
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:
	//
	// GetLockService(locEvent)->RootWriteLock();
	//  ... create historgrams or trees ...
	// GetLockService(locEvent)->RootUnLock();
	//
}

//------------------
// BeginRun
//------------------
void DEventProcessor_p2pi0_hists::BeginRun(const std::shared_ptr<const JEvent> &locEvent)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void DEventProcessor_p2pi0_hists::Process(const std::shared_ptr<const JEvent> &locEvent)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// locEvent->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	//
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// locEvent->Get(mydataclasses);
	//
	// GetLockService(locEvent)->RootWriteLock();
	//  ... fill historgrams or trees ...
	// GetLockService(locEvent)->RootUnLock();

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software

	/*********************************************************** REQUIRED ***********************************************************/

	//REQUIRED: To run an analysis, You MUST call one at least of the below code fragments. 
		//JANA is on-demand, so if you don't call one of these, then your analysis won't run. 

	/*
	//Recommended: Write surviving particle combinations (if any) to output ROOT TTree
		//If no cuts are performed by the analysis actions added to a DReaction, then this saves all of its particle combinations. 
		//The event writer gets the DAnalysisResults objects from JANA, performing the analysis. 
	// string is DReaction factory tag: will fill trees for all DReactions that are defined in the specified factory
	const DEventWriterROOT* locEventWriterROOT = NULL;
	locEvent->GetSingle(locEventWriterROOT);
	locEventWriterROOT->Fill_DataTrees(locEvent, "p2pi0_hists");
	*/

	//Optional: Get the analysis results for all DReactions. 
		//Getting these objects triggers the analysis, if it wasn't performed already. 
		//These objects contain the DParticleCombo objects that survived the DAnalysisAction cuts that were added to the DReactions
	vector<const DAnalysisResults*> locAnalysisResultsVector;
	locEvent->Get(locAnalysisResultsVector);
}

//------------------
// EndRun
//------------------
void DEventProcessor_p2pi0_hists::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void DEventProcessor_p2pi0_hists::Finish()
{
	// Called before program exit after event processing is finished.
}

