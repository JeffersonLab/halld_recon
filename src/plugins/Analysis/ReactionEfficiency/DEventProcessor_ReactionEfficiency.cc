// $Id$
//
//    File: DEventProcessor_ReactionEfficiency.cc
// Created: Wed Jun 19 16:52:58 EDT 2019
// Creator: jrsteven (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "DEventProcessor_ReactionEfficiency.h"
#include "DEventWriterROOT_ReactionEfficiency.h"

// Routine used to create our DEventProcessor

extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->Add(new DEventProcessor_ReactionEfficiency()); //register this plugin
		locApplication->Add(new DFactoryGenerator_ReactionEfficiency()); //register the factory generator
	}
} // "C"

//------------------
// Init
//------------------
void DEventProcessor_ReactionEfficiency::Init()
{
	// This is called once at program startup.
}

//------------------
// BeginRun
//------------------
void DEventProcessor_ReactionEfficiency::BeginRun(const std::shared_ptr<const JEvent>& locEvent)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void DEventProcessor_ReactionEfficiency::Process(const std::shared_ptr<const JEvent>& locEvent)
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
	// japp->RootFillLock(this);
	//  ... fill historgrams or trees ...
	// japp->RootFillUnLock(this);

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software

	/*********************************************************** REQUIRED ***********************************************************/

	//REQUIRED: To run an analysis, You MUST call one at least of the below code fragments. 
		//JANA is on-demand, so if you don't call one of these, then your analysis won't run. 

	//Recommended: Write surviving particle combinations (if any) to output ROOT TTree
		//If no cuts are performed by the analysis actions added to a DReaction, then this saves all of its particle combinations. 
		//The event writer gets the DAnalysisResults objects from JANA, performing the analysis. 
	// string is DReaction factory tag: will fill trees for all DReactions that are defined in the specified factory
	const DEventWriterROOT_ReactionEfficiency* locEventWriterROOT = NULL;
	locEvent->GetSingle(locEventWriterROOT, "ReactionEfficiency");
	locEventWriterROOT->Fill_DataTrees(locEvent, "ReactionEfficiency");

	return;
}

int DEventProcessor_ReactionEfficiency::Get_FileNumber(const std::shared_ptr<const JEvent>& locEvent) const
{
	//Assume that the file name is in the format: *_X.ext, where:
		//X is the file number (a string of numbers of any length)
		//ext is the file extension (probably .evio or .hddm)

	//get the event source
	JEventSource* locEventSource = locEvent->GetJEventSource();
	if(locEventSource == NULL)
		return -1;

	//get the source file name (strip the path)
	string locSourceFileName = GetResourceName();

	//find the last "_" & "." indices
	size_t locUnderscoreIndex = locSourceFileName.rfind("_");
	size_t locDotIndex = locSourceFileName.rfind(".");
	if((locUnderscoreIndex == string::npos) || (locDotIndex == string::npos))
		return -1;

	size_t locNumberLength = locDotIndex - locUnderscoreIndex - 1;
	string locFileNumberString = locSourceFileName.substr(locUnderscoreIndex + 1, locNumberLength);

	int locFileNumber = -1;
	istringstream locFileNumberStream(locFileNumberString);
	locFileNumberStream >> locFileNumber;

	return locFileNumber;
}

//------------------
// EndRun
//------------------
void DEventProcessor_ReactionEfficiency::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void DEventProcessor_ReactionEfficiency::Finish()
{
	// Called before program exit after event processing is finished.
	if(dEventStoreSkimStream.is_open())
		dEventStoreSkimStream.close();
}

