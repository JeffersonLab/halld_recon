// $Id$
//
//

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include "DBeamParticle_factory.h"

#include "DBeamPhoton.h"
#include "DBeamKLong.h"

#include "DVertex.h"
#include "particleType.h"
using namespace jana;

#include "DANA/DEvent.h"


//------------------
// Init
//------------------
void DBeamParticle_factory::Init() 
{
    auto app = GetApplication();

	app->SetDefaultParameter("BEAM:SKIPTAGGEDPHOTONS", dSkipTaggedPhotons,
    		"Whether to skip tagged photons in beam particle creation");
    
}

//------------------
// brun
//------------------
void DBeamParticle_factory::BeginRun(const std::shared_ptr<const JEvent>& event) 
{
    return;
}

//------------------
// evnt
//------------------
void DBeamParticle_factory::Process(const std::shared_ptr<const JEvent>& event)
{
// 	dResourcePool_BeamPhotons->Recycle(dCreated);
// 	dCreated.clear();
// 	_data.clear();

	// load beam photons
	if(!dSkipTaggedPhotons) {
		vector <const DBeamPhoton*> locBeamPhotons;
		event->Get(locBeamPhotons);
		
		for(auto locBeamPhoton : locBeamPhotons) {
			DBeamParticle* beam = new DBeamParticle(*locBeamPhoton);
			beam->dBeamPhoton = locBeamPhoton;
			Insert(beam);
		}
	}

	if(!dSkipKLongBeam) {
		// load KL beam particles
		vector <const DBeamKLong*> locBeamKLongs;
		event->Get(locBeamKLongs);
		
		for(auto locBeamKLong : locBeamKLongs) {
			DBeamParticle* beam = new DBeamParticle(*locBeamKLong);
			beam->dBeamKLong = locBeamKLong;
			Insert(beam);
		}
	}

}

