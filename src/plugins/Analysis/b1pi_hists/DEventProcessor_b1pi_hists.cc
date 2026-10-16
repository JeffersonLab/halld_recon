// $Id$
//
// File: DEventProcessor_b1pi_hists.cc
// Created: Thu Sep 28 11:38:03 EDT 2011
// Creator: pmatt (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#include "DEventProcessor_b1pi_hists.h"

// The executable should define the ROOTfile global variable. It will
// be automatically linked when dlopen is called.
extern TFile *ROOTfile;

// Routine used to create our DEventProcessor
extern "C"
{
	void InitPlugin(JApplication *app)
	{
		InitJANAPlugin(app);
		app->Add(new DEventProcessor_b1pi_hists());
		app->Add(new DFactoryGenerator_b1pi_hists());
	}
} // "C"

//------------------
// Init
//------------------
void DEventProcessor_b1pi_hists::Init()
{
}

//------------------
// BeginRun
//------------------
void DEventProcessor_b1pi_hists::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DEventProcessor_b1pi_hists::Process(const std::shared_ptr<const JEvent>& locEvent)
{
	//Triggers the analysis (is also automatically called by DEventWriterROOT::Fill_Trees())
	vector<const DAnalysisResults*> locAnalysisResultsVector;
	locEvent->Get(locAnalysisResultsVector);

	//Output TTree
	const DEventWriterROOT* locEventWriterROOT = NULL;
	locEvent->GetSingle(locEventWriterROOT);
	locEventWriterROOT->Fill_DataTrees(locEvent, "b1pi_hists");

	//Do Miscellaneous Cuts
	bool locSaveEventFlag = false;
	for(size_t loc_i = 0; loc_i < locAnalysisResultsVector.size(); ++loc_i)
	{
		const DAnalysisResults* locAnalysisResults = locAnalysisResultsVector[loc_i];
		if(locAnalysisResults->Get_Reaction()->Get_ReactionName() != "b1pi")
			continue;
		if(locAnalysisResults->Get_NumPassedParticleCombos() == 0)
			continue;
		locSaveEventFlag = true;
		break;
	}

	//Output REST File
	if(locSaveEventFlag)
	{
		vector<const DEventWriterREST*> locEventWriterRESTVector;
		locEvent->Get(locEventWriterRESTVector);
		locEventWriterRESTVector[0]->Write_RESTEvent(locEvent, "b1pi");
	}
}

//------------------
// EndRun
//------------------
void DEventProcessor_b1pi_hists::EndRun()
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
}

//------------------
// Finish
//------------------
void DEventProcessor_b1pi_hists::Finish()
{
}

