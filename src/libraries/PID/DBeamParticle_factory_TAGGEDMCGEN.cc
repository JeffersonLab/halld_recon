// $Id$
//
//

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include "DBeamParticle_factory_TAGGEDMCGEN.h"

#include "DBeamPhoton.h"
#include "DBeamKLong.h"

#include "DVertex.h"
#include "particleType.h"
using namespace jana;

#include "DANA/DEvent.h"


//------------------
// Init
//------------------
void DBeamParticle_factory_TAGGEDMCGEN::Init() 
{
    auto app = GetApplication();

	app->SetDefaultParameter("BEAM:SKIPTAGGEDPHOTONS", dSkipTaggedPhotons,
    		"Whether to skip tagged photons in beam particle creation");
    
}

//------------------
// brun
//------------------
void DBeamParticle_factory_TAGGEDMCGEN::BeginRun(const std::shared_ptr<const JEvent>& event) 
{
    return;
}

//------------------
// evnt
//------------------
void DBeamParticle_factory_TAGGEDMCGEN::Process(const std::shared_ptr<const JEvent>& event)
{
	// load beam photons
	if(!dSkipTaggedPhotons) {
		vector <const DBeamPhoton*> locBeamPhotons;
		event->Get(locBeamPhotons, "MCGEN");
		
		for(auto locBeamPhoton : locBeamPhotons) {
			DBeamParticle* beam = new DBeamParticle(*locBeamPhoton);
			beam->dBeamPhoton = locBeamPhoton;
			Insert(beam);
		}
	}

	// KL beam has no tagging, so no objects
}

