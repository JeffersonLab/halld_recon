// $Id$
//
//    File: DNeutralParticle_factory_PreSelect.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: staylor (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DNeutralParticle_factory_PreSelect.h"

//------------------
// init
//------------------
jerror_t DNeutralParticle_factory_PreSelect::init(void)
{
	//Setting this flag makes it so that JANA does not delete the objects in _data.  This factory will manage this memory. 
	//This is because some/all of these pointers are just copied from earlier objects, and should not be deleted.  
	SetFactoryFlag(NOT_OBJECT_OWNER);
	dResourcePool_NeutralParticle = new DResourcePool<DNeutralParticle>();
	dResourcePool_NeutralParticle->Set_ControlParams(50, 20, 400, 4000, 0);

	// default selections
	// allow for the application of a beta cut for neutrons to reduce combinations
	// but this should be LOOSE since the velocity calculation depends strongly on
	// the exact vertex, and at this point we don't know that so have to assume
	// the center of the target as a default
	dMaxNeutronBeta = 1.0;   // don't throw anything away by default

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DNeutralParticle_factory_PreSelect::brun(jana::JEventLoop *locEventLoop, int32_t runnumber)
{
	gPARMS->SetDefaultParameter("PRESELECT:MAX_NEUTRON_BETA", dMaxNeutronBeta);   

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DNeutralParticle_factory_PreSelect::evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber)
{
	//Clear objects from last event
	dResourcePool_NeutralParticle->Recycle(dCreated);
	dCreated.clear();
	_data.clear();

	vector<const DNeutralParticle*> locNeutralParticles;
	locEventLoop->Get(locNeutralParticles);

	vector<const DNeutralShower*> locNeutralShowers;
	locEventLoop->Get(locNeutralShowers, "PreSelect");

	set<const DNeutralShower*> locNeutralShowerSet;
	for(size_t loc_i = 0; loc_i < locNeutralShowers.size(); ++loc_i)
		locNeutralShowerSet.insert(locNeutralShowers[loc_i]);

	for(size_t loc_i = 0; loc_i < locNeutralParticles.size(); ++loc_i)
	{
		 DNeutralParticle* locNeutralParticle_PreSelected = new DNeutralParticle(*locNeutralParticles[loc_i]);
	
		//if neutral shower was good, keep particle, else ignore it
		if(locNeutralShowerSet.find(locNeutralParticle_PreSelected->dNeutralShower) == locNeutralShowerSet.end())
			continue;
		
		// make extra selections for particular hypotheses
		auto locHypothesisItr = locNeutralParticle_PreSelected->dNeutralParticleHypotheses.begin();
		while( locHypothesisItr != locNeutralParticle_PreSelected->dNeutralParticleHypotheses.end()) {
			// extra selections for neutrons
			if((*locHypothesisItr)->PID() == Neutron) {
				if((*locHypothesisItr)->measuredBeta() > dMaxNeutronBeta) {
					locHypothesisItr = locNeutralParticle_PreSelected->dNeutralParticleHypotheses.erase(locHypothesisItr);  // delete and move to next
					continue;
				}
			}
			
			locHypothesisItr++;   // move to next element
		}
		
		// keep the shower
		_data.push_back(const_cast<DNeutralParticle*>(locNeutralParticle_PreSelected));
	}

	dCreated = _data;
	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DNeutralParticle_factory_PreSelect::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DNeutralParticle_factory_PreSelect::fini(void)
{
	for(auto locHypo : _data)
		Recycle_Hypothesis(locHypo);
	_data.clear();
	delete dResourcePool_NeutralParticle;

	return NOERROR;
}


