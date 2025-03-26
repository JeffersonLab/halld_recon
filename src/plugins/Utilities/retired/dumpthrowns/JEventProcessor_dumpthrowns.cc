// $Id$
//
//    File: JEventProcessor_dumpthrowns.cc
// Created: Tue Feb  4 09:29:35 EST 2014
// Creator: davidl (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_dumpthrowns.h"


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>

#include <TRACKING/DTrackCandidate.h>
#include <TRACKING/DMCThrown.h>

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_dumpthrowns());
}
} // "C"


//------------------
// JEventProcessor_dumpthrowns (Constructor)
//------------------
JEventProcessor_dumpthrowns::JEventProcessor_dumpthrowns()
{
	SetTypeName("JEventProcessor_dumpthrowns");

	events_written = 0;
	events_discarded = 0;
}

//------------------
// ~JEventProcessor_dumpthrowns (Destructor)
//------------------
JEventProcessor_dumpthrowns::~JEventProcessor_dumpthrowns()
{
}

//------------------
// Init
//------------------
void JEventProcessor_dumpthrowns::Init()
{
	auto app = GetApplication();
	lockService = app->GetService<JLockService>();

	MAX_CANDIDATE_FILTER = 1000;
	app->SetDefaultParameter("MAX_CANDIDATE_FILTER", MAX_CANDIDATE_FILTER, "Maximum number of candidates allowed in event before any are written to file.");

	// Open output file
	ofs = new ofstream("gluex_throwns.txt");
}

//------------------
// BeginRun
//------------------
void JEventProcessor_dumpthrowns::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void JEventProcessor_dumpthrowns::Process(const std::shared_ptr<const JEvent>& event)
{
	// Get track candidates
	vector<const DTrackCandidate*> candidates;
	event->Get(candidates);
	if(candidates.size()==0 || candidates.size()>MAX_CANDIDATE_FILTER){
		events_discarded++;
		return;
	}
	
	// Get thrown particles
	vector<const DMCThrown*> throwns;
	event->Get(throwns);

	// Write out thrown parameters	
	for(unsigned int i=0; i<throwns.size(); i++){
		const DMCThrown *thrown = throwns[i];
		
		// Write thrown parameters to string
		stringstream ss;
		ss << thrown->charge();
		ss << " " << thrown->x() << " " << thrown->y() << " " << thrown->z();
		ss << " " << thrown->px() << " " << thrown->py() << " " << thrown->pz();
		
		// Write thrown parameters string to file
		lockService->RootFillLock(this);
		(*ofs) << ss.str() << endl;
		events_written++;
		lockService->RootFillUnLock(this);

		// Sometimes, generated particles are added to the thrown
		// particles list. We want only the first MAX_CANDIDATE_FILTER
		// particles which *may* correspond to the first candidates.
		// (probably not, but at least it may work for the single particle
		// case which is what we are interested in at the moment).
		if((i+1) >= MAX_CANDIDATE_FILTER) break;
	}
}

//------------------
// EndRun
//------------------
void JEventProcessor_dumpthrowns::EndRun()
{
}

//------------------
// Finish
//------------------
void JEventProcessor_dumpthrowns::Finish()
{
	if(ofs){
		ofs->close();
		delete ofs;
		ofs =NULL;
	}
	
	cout << endl;
	cout << "Wrote " << events_written << " thrown events to output file (discarded " << events_discarded << ")" << endl;
	cout << endl;
}



