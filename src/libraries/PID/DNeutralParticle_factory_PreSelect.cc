// $Id$
//
//    File: DNeutralParticle_factory_PreSelect.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: staylor (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DNeutralParticle_factory_PreSelect.h"
#include <JANA/JEvent.h>

//------------------
// Init
//------------------
void DNeutralParticle_factory_PreSelect::Init()
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
}

//------------------
// BeginRun
//------------------
void DNeutralParticle_factory_PreSelect::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	GetApplication()->SetDefaultParameter("PRESELECT:MAX_NEUTRON_BETA", dMaxNeutronBeta);
}

//------------------
// Process
//------------------
void DNeutralParticle_factory_PreSelect::Process(const std::shared_ptr<const JEvent>& event)
{
	//Clear objects from last event
	dResourcePool_NeutralParticle->Recycle(dCreated);
	dCreated.clear();
	mData.clear();
	// TODO: NWB: Aesthetics: Improve getter to mData

	vector<const DNeutralParticle*> locNeutralParticles;
	event->Get(locNeutralParticles);

	vector<const DNeutralShower*> locNeutralShowers;
	event->Get(locNeutralShowers, "PreSelect");
	vector<const DNeutralShower*> locHadronNeutralShowers;
	event->Get(locHadronNeutralShowers, "HadronPreSelect");

	// 
	set<const DNeutralShower*> locNeutralShowerSet;
	for(size_t loc_i = 0; loc_i < locNeutralShowers.size(); ++loc_i)
		locNeutralShowerSet.insert(locNeutralShowers[loc_i]);
	set<const DNeutralShower*> locHadronNeutralShowersSet;
	for(size_t loc_i = 0; loc_i < locHadronNeutralShowers.size(); ++loc_i)
		locHadronNeutralShowersSet.insert(locHadronNeutralShowers[loc_i]);

	// apply PreSelect selections on neutral particles
	for(size_t loc_i = 0; loc_i < locNeutralParticles.size(); ++loc_i)
	{
		 DNeutralParticle* locNeutralParticle_PreSelected = new DNeutralParticle(*locNeutralParticles[loc_i]);
	
		// make extra selections for particular hypotheses
		auto locHypothesisItr = locNeutralParticle_PreSelected->dNeutralParticleHypotheses.begin();
		while( locHypothesisItr != locNeutralParticle_PreSelected->dNeutralParticleHypotheses.end()) {

			if((*locHypothesisItr)->PID() == Gamma) {
				//if neutral shower was good, keep hypothesis, else ignore it
				if(locNeutralShowerSet.find(locNeutralParticle_PreSelected->dNeutralShower) == locNeutralShowerSet.end()) {
					locHypothesisItr = locNeutralParticle_PreSelected->dNeutralParticleHypotheses.erase(locHypothesisItr);  // delete and move to next
					continue;
				}
				
			} else if((*locHypothesisItr)->PID() == Neutron) {
				//if neutral shower was good, keep hypothesis, else ignore it
				if(locHadronNeutralShowersSet.find(locNeutralParticle_PreSelected->dNeutralShower) == locHadronNeutralShowersSet.end()) {
					locHypothesisItr = locNeutralParticle_PreSelected->dNeutralParticleHypotheses.erase(locHypothesisItr);  // delete and move to next
					continue;
				}

				// allow for a LOOSE beta cut
				if((*locHypothesisItr)->measuredBeta() > dMaxNeutronBeta) {
					locHypothesisItr = locNeutralParticle_PreSelected->dNeutralParticleHypotheses.erase(locHypothesisItr);  // delete and move to next
					continue;
				}
			}
			
			locHypothesisItr++;   // move to next element
		}
		
		// keep the particle if any of the hypotheses survive
		if(locNeutralParticle_PreSelected->dNeutralParticleHypotheses.size() > 0)
			Insert(const_cast<DNeutralParticle*>(locNeutralParticle_PreSelected));
		else 
			delete locNeutralParticle_PreSelected;
	}

	dCreated = mData;
}

//------------------
// EndRun
//------------------
void DNeutralParticle_factory_PreSelect::EndRun()
{
}

//------------------
// Finish
//------------------
void DNeutralParticle_factory_PreSelect::Finish()
{
	for(auto locHypo : mData)
		Recycle_Hypothesis(locHypo);
	mData.clear();
	delete dResourcePool_NeutralParticle;
}


