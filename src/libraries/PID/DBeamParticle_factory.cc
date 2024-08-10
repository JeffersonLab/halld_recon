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


//------------------
// init
//------------------
jerror_t DBeamParticle_factory::init(void)
{

	//Setting this flag makes it so that JANA does not delete the objects in _data.  This factory will manage this memory.
	SetFactoryFlag(NOT_OBJECT_OWNER);
	return NOERROR;
	
	gPARMS->SetDefaultParameter("BEAM:SKIPTAGGEDPHOTONS", dSkipTaggedPhotons,
    "Whether to skip tagged photons in beam particle creation");
    
}

//------------------
// brun
//------------------
jerror_t DBeamParticle_factory::brun(jana::JEventLoop *locEventLoop, int32_t runnumber)
{

    return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DBeamParticle_factory::evnt(jana::JEventLoop *locEventLoop, uint64_t locEventNumber)
{
// 	dResourcePool_BeamPhotons->Recycle(dCreated);
// 	dCreated.clear();
// 	_data.clear();

	// load beam photons
	if(!dSkipTaggedPhotons) {
		vector <const DBeamPhoton*> locBeamPhotons;
		locEventLoop->Get(locBeamPhotons);
		
		for(auto locBeamPhoton : locBeamPhotons) {
			DBeamParticle* beam = new DBeamParticle(*locBeamPhoton);
			beam->dBeamPhoton = locBeamPhoton;
			_data.push_back(beam);
		}
	}

	// load KL beam particles
	vector <const DBeamKLong*> locBeamKLongs;
	locEventLoop->Get(locBeamKLongs);
	
	for(auto locBeamKLong : locBeamKLongs) {
		DBeamParticle* beam = new DBeamParticle(*locBeamKLong);
		beam->dBeamKLong = locBeamKLong;
		_data.push_back(beam);
	}


    return NOERROR;
}

