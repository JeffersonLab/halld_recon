// $Id$
//
//    File: DMCThrown_factory_Primary.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DMCThrown_factory_Primary.h"



//------------------
// Init
//------------------
void DMCThrown_factory_Primary::Init()
{
	SetFactoryFlag(NOT_OBJECT_OWNER);
}

//------------------
// BeginRun
//------------------
void DMCThrown_factory_Primary::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	event->GetSingle(dAnalysisUtilities);
}

//------------------
// Process
//------------------
void DMCThrown_factory_Primary::Process(const std::shared_ptr<const JEvent>& event)
{
	mData.clear();

	deque<pair<const DMCThrown*, deque<const DMCThrown*> > > locThrownSteps;
	dAnalysisUtilities->Get_ThrownParticleSteps(event, locThrownSteps);

	if(locThrownSteps.empty())
		return;

	deque<const DMCThrown*>& locParticles = locThrownSteps[0].second;
	for(size_t loc_i = 0; loc_i < locParticles.size(); ++loc_i)
		mData.push_back(const_cast<DMCThrown*>(locParticles[loc_i]));
}


//------------------
// EndRun
//------------------
void DMCThrown_factory_Primary::EndRun()
{
}

//------------------
// Finish
//------------------
void DMCThrown_factory_Primary::Finish()
{
}

