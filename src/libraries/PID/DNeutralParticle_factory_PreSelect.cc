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

	// default selections
	// NOTE: disabling this for now, will try to handle all event selections in ANALYSIS library
	// since beta depends on the vertex.  If we go back and try to do some loose preselections here,
	// then we'll want to not just copy pointers, but manage our own memory so that we can 
	// decide if we want to keep one hypothesis but not the other
	//dMaxNeutronBeta = 0.9;

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DNeutralParticle_factory_PreSelect::brun(jana::JEventLoop *locEventLoop, int32_t runnumber)
{
	//gPARMS->SetDefaultParameter("PRESELECT:MAX_NEUTRON_BETA", dMaxNeutronBeta);   

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DNeutralParticle_factory_PreSelect::evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber)
{
	//Clear objects from last event
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
		//if neutral shower was good, keep particle, else ignore it
		if(locNeutralShowerSet.find(locNeutralParticles[loc_i]->dNeutralShower) == locNeutralShowerSet.end())
			continue;
		
		// extra selections for neutrons
		//if(locNeutralParticles[loc_i]->Get_PID() == Neutron) {
		//	if(locNeutralParticles[loc_i]->measuredBeta() > dMaxNeutronBeta)
		//		continue;
		//}
		
		// keep the shower
		_data.push_back(const_cast<DNeutralParticle*>(locNeutralParticles[loc_i]));
	}

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
	return NOERROR;
}


