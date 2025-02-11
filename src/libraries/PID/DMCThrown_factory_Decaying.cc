// $Id$
//
//    File: DMCThrown_factory_Decaying.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DMCThrown_factory_Decaying.h"



//------------------
// Init
//------------------
void DMCThrown_factory_Decaying::Init()
{
	SetFactoryFlag(NOT_OBJECT_OWNER);
}

//------------------
// BeginRun
//------------------
void DMCThrown_factory_Decaying::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	event->GetSingle(dAnalysisUtilities);
}

//------------------
// Process
//------------------
void DMCThrown_factory_Decaying::Process(const std::shared_ptr<const JEvent>& event)
{
	mData.clear();

	deque<pair<const DMCThrown*, deque<const DMCThrown*> > > locThrownSteps;
	dAnalysisUtilities->Get_ThrownParticleSteps(event, locThrownSteps);

	if(locThrownSteps.empty())
		return;

	for(size_t loc_i = 1; loc_i < locThrownSteps.size(); ++loc_i)
		mData.push_back(const_cast<DMCThrown*>(locThrownSteps[loc_i].first));
}


//------------------
// EndRun
//------------------
void DMCThrown_factory_Decaying::EndRun()
{
}

//------------------
// Finish
//------------------
void DMCThrown_factory_Decaying::Finish()
{
}

