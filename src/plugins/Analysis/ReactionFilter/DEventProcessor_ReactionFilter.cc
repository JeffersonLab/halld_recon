// $Id$
//
//    File: DEventProcessor_ReactionFilter.cc
// Created: Mon Nov 21 17:54:40 EST 2016
// Creator: pmatt (on Darwin Pauls-MacBook-Pro-2.local 13.4.0 i386)
//

#include "DEventProcessor_ReactionFilter.h"
#include <JANA/JEventSource.h>

// Routine used to create our DEventProcessor

extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->Add(new DEventProcessor_ReactionFilter()); //register this plugin
		locApplication->Add(new DFactoryGenerator_ReactionFilter()); //register the factory generator
	}
} // "C"

//------------------
// Init
//------------------
void DEventProcessor_ReactionFilter::Init()
{
	// This is called once at program startup.

	/*
	//OPTIONAL: Create an EventStore skim.  
	string locSkimFileName = "ReactionFilter.idxa";
	dEventStoreSkimStream.open(locSkimFileName.c_str());
	dEventStoreSkimStream << "IDXA" << endl;
	*/
}

//------------------
// BeginRun
//------------------
void DEventProcessor_ReactionFilter::BeginRun(const std::shared_ptr<const JEvent> &locEvent)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void DEventProcessor_ReactionFilter::Process(const std::shared_ptr<const JEvent> &locEvent)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// locEvent->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software

	/*********************************************************** REQUIRED ***********************************************************/

	//REQUIRED: To run an analysis, You MUST call one at least of the below code fragments. 
		//JANA is on-demand, so if you don't call one of these, then your analysis won't run. 

	//Recommended: Write surviving particle combinations (if any) to output ROOT TTree
		//If no cuts are performed by the analysis actions added to a DReaction, then this saves all of its particle combinations. 
		//The event writer gets the DAnalysisResults objects from JANA, performing the analysis. 
	// string is DReaction factory tag: will fill trees for all DReactions that are defined in the specified factory
  
	const DEventWriterROOT* locEventWriterROOT = NULL;
	locEvent->GetSingle(locEventWriterROOT);
	locEventWriterROOT->Fill_DataTrees(locEvent, "ReactionFilter");
  
	/******************************************************** OPTIONAL: SKIMS *******************************************************/

	/*
	//Optional: Create an EventStore skim. 

	// See whether this is MC data or real data
	vector<const DMCThrown*> locMCThrowns;
	locEvent->Get(locMCThrowns);

	unsigned int locRunNumber = locEvent->GetJEvent().GetRunNumber();
	unsigned int locUniqueID = locMCThrowns.empty() ? 1 : Get_FileNumber(locEvent);

	// If a particle combo passed the cuts, save the event info in the output file
	for(size_t loc_i = 0; loc_i < locAnalysisResultsVector.size(); ++loc_i)
	{
		const DAnalysisResults* locAnalysisResults = locAnalysisResultsVector[loc_i];
		if(locAnalysisResults->Get_Reaction()->Get_ReactionName() != "ReactionFilter")
			continue; // analysis results were for a different reaction
		if(locAnalysisResults->Get_NumPassedParticleCombos() == 0)
			continue; // no combos passed

		//MUST LOCK AROUND MODIFICATION OF MEMBER VARIABLES IN brun() or evnt().
		japp->WriteLock("ReactionFilter.idxa"); //Lock is unique to this output file
		{
			dEventStoreSkimStream << locRunNumber << " " << locEventNumber << " " << locUniqueID << endl;
		}
		japp->Unlock("ReactionFilter.idxa");
	}
	*/
}

int DEventProcessor_ReactionFilter::Get_FileNumber(const std::shared_ptr<const JEvent>& locEvent) const
{
	//Assume that the file name is in the format: *_X.ext, where:
		//X is the file number (a string of numbers of any length)
		//ext is the file extension (probably .evio or .hddm)

	//get the event source
	JEventSource* locEventSource = locEvent->GetJEventSource();
	if(locEventSource == NULL)
		return -1;

	//get the source file name (strip the path)
	string locSourceFileName = locEventSource->GetResourceName();

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
void DEventProcessor_ReactionFilter::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void DEventProcessor_ReactionFilter::Finish()
{
	// Called before program exit after event processing is finished.
	if(dEventStoreSkimStream.is_open())
		dEventStoreSkimStream.close();
}

