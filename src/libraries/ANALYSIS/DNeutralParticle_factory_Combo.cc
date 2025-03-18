#include "DNeutralParticle_factory_Combo.h"

//------------------
// Init
//------------------
void DNeutralParticle_factory_Combo::Init()
{
	//Get preselect tag
	auto app = GetApplication();
	dShowerSelectionTag = "PreSelect";
	app->SetDefaultParameter("COMBO:SHOWER_SELECT_TAG", dShowerSelectionTag);
}

//------------------
// BeginRun
//------------------
void DNeutralParticle_factory_Combo::BeginRun(const std::shared_ptr<const JEvent>& locEvent)
{
	vector<const DNeutralParticleHypothesis*> locNeutralParticleHypotheses;
	locEvent->Get(locNeutralParticleHypotheses); //make sure that brun() is called for the default factory!!!
	dNeutralParticleHypothesisFactory = dynamic_cast<DNeutralParticleHypothesis_factory*>(locEvent->GetFactory<DNeutralParticleHypothesis>());
	// TODO: NWB: Don't do this!

	//Get Needed PIDs
	auto locReactions = DAnalysis::Get_Reactions(locEvent);
	for(size_t loc_i = 0; loc_i < locReactions.size(); ++loc_i)
	{
		auto locNeutralPIDs = locReactions[loc_i]->Get_FinalPIDs(-1, false, false, d_Neutral, false);
		for(auto locPID : locNeutralPIDs)
		{
			if( (locPID != Gamma) && (locPID != Neutron) ) //already created by default!
				dNeutralPIDs.insert(locPID);
		}
	}

	//Setting this flag makes it so that JANA does not delete the objects in _data.  This factory will manage this memory.
	if(dNeutralPIDs.empty())
		SetFactoryFlag(NOT_OBJECT_OWNER);
}

//------------------
// Process
//------------------
void DNeutralParticle_factory_Combo::Process(const std::shared_ptr<const JEvent>& locEvent)
{
	vector<const DNeutralParticle*> locNeutralParticles;
	locEvent->Get(locNeutralParticles, dShowerSelectionTag.c_str());

	//Nothing to do! Pass them through
	if(dNeutralPIDs.empty())
	{
		mData.clear();
		for(auto& locNeutralParticle : locNeutralParticles)
			mData.push_back(const_cast<DNeutralParticle*>(locNeutralParticle));
		return;
	}

	const DEventRFBunch* locEventRFBunch = nullptr;
	locEvent->GetSingle(locEventRFBunch);

	const DVertex* locVertex = nullptr;
	locEvent->GetSingle(locVertex);

	dNeutralParticleHypothesisFactory->Recycle_Hypotheses(dCreatedHypotheses);

	//create and add new hypos
	for(auto& locNeutralParticle : locNeutralParticles)
	{
		auto locNewNeutralParticle = new DNeutralParticle(*locNeutralParticle);
		for(auto& locPID : dNeutralPIDs)
		{
			//create new DNeutralParticleHypothesis object
			auto locNewHypothesis = dNeutralParticleHypothesisFactory->Create_DNeutralParticleHypothesis(locNeutralParticle->dNeutralShower, locPID, locEventRFBunch, locVertex->dSpacetimeVertex, &locVertex->dCovarianceMatrix);
			if(locNewHypothesis == nullptr)
				continue;
			dCreatedHypotheses.push_back(locNewHypothesis);
			locNewNeutralParticle->dNeutralParticleHypotheses.push_back(locNewHypothesis);
		}
		Insert(locNewNeutralParticle);
	}
}
