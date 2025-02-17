// $Id$
//
//    File: DEventRFBunch_factory_Thrown.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DEventRFBunch_factory_Thrown.h"
#include <deque>
#include <JANA/JEvent.h>



//------------------
// Init
//------------------
void DEventRFBunch_factory_Thrown::Init()
{
}

//------------------
// BeginRun
//------------------
void DEventRFBunch_factory_Thrown::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DEventRFBunch_factory_Thrown::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DMCThrown*> locMCThrowns;
	event->Get(locMCThrowns);
	if(locMCThrowns.empty())
		return; //not a MC event!

	vector<const DRFTime*> locRFTimes;
	event->Get(locRFTimes, "TRUTH");
	if(locRFTimes.empty())
	{
		DEventRFBunch *locEventRFBunch = new DEventRFBunch;
		locEventRFBunch->dTime = numeric_limits<double>::quiet_NaN();
		locEventRFBunch->dTimeVariance = numeric_limits<double>::quiet_NaN();
		locEventRFBunch->dNumParticleVotes = 0;
		locEventRFBunch->dTimeSource = SYS_NULL;
		Insert(locEventRFBunch);
		return;
	}

	DEventRFBunch *locEventRFBunch = new DEventRFBunch;
	locEventRFBunch->dTime = locRFTimes[0]->dTime;
	locEventRFBunch->dTimeVariance = 0.0;
	locEventRFBunch->dNumParticleVotes = 0;
	locEventRFBunch->dTimeSource = SYS_RF;
	Insert(locEventRFBunch);
}

//------------------
// EndRun
//------------------
void DEventRFBunch_factory_Thrown::EndRun()
{
}

//------------------
// Finish
//------------------
void DEventRFBunch_factory_Thrown::Finish()
{
}

