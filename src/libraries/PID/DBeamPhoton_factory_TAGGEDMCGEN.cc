// $Id$
//
//    File: DBeamPhoton_factory_TAGGEDMCGEN.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: staylor (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include <iostream>
#include <iomanip>
using namespace std;

#include "DBeamPhoton_factory_TAGGEDMCGEN.h"

#include <JANA/JEvent.h>


//------------------
// Process
//------------------
void DBeamPhoton_factory_TAGGEDMCGEN::Process(const std::shared_ptr<const JEvent>& event)
{
	mData.clear();

	//Check if MC
	vector<const DMCReaction*> locMCReactions;
	event->Get(locMCReactions);
	if(locMCReactions.empty())
		return; //Not a thrown event

	//Get the MCGEN beam
	const DBeamPhoton* locMCGenBeam;
	event->GetSingle(locMCGenBeam, "MCGEN");

	//See if it was tagged
	auto locSystem = locMCGenBeam->dSystem;
	if(locSystem == SYS_NULL)
		return; //Nope, no objects to create

	//Get reconstructed beam photons
	vector<const DBeamPhoton*> locBeamPhotons;
	event->Get(locBeamPhotons);

	//Loop over beam photons
	double locBestDeltaT = 9.9E9;
	const DBeamPhoton* locBestPhoton = nullptr;
	for(auto& locBeamPhoton : locBeamPhotons)
	{
		if(locBeamPhoton->dSystem != locSystem)
			continue;
		if(locBeamPhoton->dCounter != locMCGenBeam->dCounter)
			continue;

		auto locDeltaT = fabs(locMCGenBeam->time() - locBeamPhoton->time());
		if(locDeltaT >= locBestDeltaT)
			continue;
		locBestDeltaT = locDeltaT;
		locBestPhoton = locBeamPhoton;
	}

	if(locBestPhoton == nullptr)
		return; //Uh oh.  Shouldn't be possible. 

	mData.push_back(new DBeamPhoton(*locBestPhoton));
}

