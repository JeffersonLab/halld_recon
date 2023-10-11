// $Id$
//
//    File: DNeutralParticle_factory.cc
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DNeutralParticle_factory.h"
#include "DResourcePool.h"

#include <JANA/JEvent.h>


//------------------
// Init
//------------------
void DNeutralParticle_factory::Init()
{
}

//------------------
// BeginRun
//------------------
void DNeutralParticle_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DNeutralParticle_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DNeutralParticleHypothesis*> locNeutralParticleHypotheses;
	event->Get(locNeutralParticleHypotheses);

	map<const DNeutralShower*, vector<const DNeutralParticleHypothesis*> > locHypothesesByShower;
	for(size_t loc_i = 0; loc_i < locNeutralParticleHypotheses.size(); loc_i++)
		locHypothesesByShower[locNeutralParticleHypotheses[loc_i]->Get_NeutralShower()].push_back(locNeutralParticleHypotheses[loc_i]);

	for(auto& locPair : locHypothesesByShower)
	{
		DNeutralParticle* locNeutralParticle = new DNeutralParticle();
		locNeutralParticle->dNeutralShower = locPair.first;
		locNeutralParticle->dNeutralParticleHypotheses = locPair.second;
		Insert(locNeutralParticle);
	}
}

//------------------
// EndRun
//------------------
void DNeutralParticle_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DNeutralParticle_factory::Finish()
{
}


