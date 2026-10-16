// $Id$
//
//    File: DMCThrown_factory_FinalState.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DMCThrown_factory_FinalState.h"



//------------------
// Init
//------------------
void DMCThrown_factory_FinalState::Init()
{
	SetFactoryFlag(NOT_OBJECT_OWNER);
}

//------------------
// BeginRun
//------------------
void DMCThrown_factory_FinalState::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	event->GetSingle(dAnalysisUtilities);
}

//------------------
// Process
//------------------
void DMCThrown_factory_FinalState::Process(const std::shared_ptr<const JEvent>& event)
{
	mData.clear();

	deque<pair<const DMCThrown*, deque<const DMCThrown*> > > locThrownSteps;
	dAnalysisUtilities->Get_ThrownParticleSteps(event, locThrownSteps);

	if(locThrownSteps.empty())
		return;

	for(size_t loc_i = 0; loc_i < locThrownSteps.size(); ++loc_i)
	{
		deque<const DMCThrown*>& locParticles = locThrownSteps[loc_i].second;
		for(size_t loc_j = 0; loc_j < locParticles.size(); ++loc_j)
		{
			if(Is_FinalStateParticle(locParticles[loc_j]->PID()))
				mData.push_back(const_cast<DMCThrown*>(locParticles[loc_j]));
		}
	}
}


//------------------
// EndRun
//------------------
void DMCThrown_factory_FinalState::EndRun()
{
}

//------------------
// Finish
//------------------
void DMCThrown_factory_FinalState::Finish()
{
}

