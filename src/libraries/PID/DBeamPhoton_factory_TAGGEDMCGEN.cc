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

	//Get every primary (bg == 0) truth-tagged system for the thrown photon.
	//In TAGH/TAGM overlap regions the photon can legitimately register truth
	//hits in BOTH systems (see DBeamPhoton_factory_TRUTH), just as a real
	//photon in that energy range produces coincident TAGH+TAGM hits and
	//un-merged reconstructed DBeamPhoton candidates (see DBeamPhoton_factory).
	//"MCGEN" only reports a single canonical system (preferring TAGM) since
	//it exists to represent the one physical photon's kinematics, not to
	//enumerate which tagger systems it was tagged in - so it can't be used
	//here without silently dropping the second, equally real, truth match.
	vector<const DBeamPhoton*> locTruthPhotons;
	event->Get(locTruthPhotons, "TRUTH");

	//Get reconstructed beam photons
	vector<const DBeamPhoton*> locBeamPhotons;
	event->Get(locBeamPhotons);

	for(auto& locTruthPhoton : locTruthPhotons)
	{
		//only consider the primary photon's own hits, not accidentals/pileup
		bool locIsPrimary = false;
		vector<const DTAGMHit*> locTAGMHits;
		locTruthPhoton->Get(locTAGMHits);
		for(auto locTAGMHit : locTAGMHits)
		{
			if(locTAGMHit->bg == 0)
				locIsPrimary = true;
		}
		vector<const DTAGHHit*> locTAGHHits;
		locTruthPhoton->Get(locTAGHHits);
		for(auto locTAGHHit : locTAGHHits)
		{
			if(locTAGHHit->bg == 0)
				locIsPrimary = true;
		}
		if(!locIsPrimary)
			continue;

		auto locSystem = locTruthPhoton->dSystem;
		if(locSystem == SYS_NULL)
			continue;

		//Loop over beam photons
		double locBestDeltaT = 9.9E9;
		const DBeamPhoton* locBestPhoton = nullptr;
		for(auto& locBeamPhoton : locBeamPhotons)
		{
			if(locBeamPhoton->dSystem != locSystem)
				continue;
			if(locBeamPhoton->dCounter != locTruthPhoton->dCounter)
				continue;

			auto locDeltaT = fabs(locTruthPhoton->time() - locBeamPhoton->time());
			if(locDeltaT >= locBestDeltaT)
				continue;
			locBestDeltaT = locDeltaT;
			locBestPhoton = locBeamPhoton;
		}

		if(locBestPhoton != nullptr)
			mData.push_back(new DBeamPhoton(*locBestPhoton));
	}
}

