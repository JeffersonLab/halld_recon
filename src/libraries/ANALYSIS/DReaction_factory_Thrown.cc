// $Id$
//
//    File: DReaction_factory_Thrown.cc
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt
//

#ifdef VTRACE
#include "vt_user.h"
#endif

#include "DReaction_factory_Thrown.h"

//------------------
// Init
//------------------
void DReaction_factory_Thrown::Init()
{
	MAX_dReactionStepPoolSize = 5;
}

//------------------
// BeginRun
//------------------
void DReaction_factory_Thrown::BeginRun(const std::shared_ptr<const JEvent>& locEvent)
{
 	vector<const DAnalysisUtilities*> locAnalysisUtilitiesVector;
	locEvent->Get(locAnalysisUtilitiesVector);
	dAnalysisUtilities = locAnalysisUtilitiesVector[0];
}

//------------------
// Process
//------------------
void DReaction_factory_Thrown::Process(const std::shared_ptr<const JEvent>& locEvent)
{
	// delete pool sizes if too large, preventing memory-leakage-like behavor.
	if(dReactionStepPool_All.size() > MAX_dReactionStepPoolSize){
		for(size_t loc_i = MAX_dReactionStepPoolSize; loc_i < dReactionStepPool_All.size(); ++loc_i)
			delete dReactionStepPool_All[loc_i];
		dReactionStepPool_All.resize(MAX_dReactionStepPoolSize);
	}
	dReactionStepPool_Available = dReactionStepPool_All;

	deque<pair<const DMCThrown*, deque<const DMCThrown*> > > locThrownSteps;
	dAnalysisUtilities->Get_ThrownParticleSteps(locEvent, locThrownSteps);

	DReaction* locReaction = Build_ThrownReaction(locEvent, locThrownSteps);
	Insert(locReaction);
}

DReaction* DReaction_factory_Thrown::Build_ThrownReaction(const std::shared_ptr<const JEvent>& locEvent, deque<pair<const DMCThrown*, deque<const DMCThrown*> > >& locThrownSteps)
{
#ifdef VTRACE
	VT_TRACER("DReaction_factory_Thrown::Build_ThrownReaction()");
#endif

 	vector<const DMCReaction*> locMCReactions;
	locEvent->Get(locMCReactions);

	DReaction* locReaction = new DReaction("Thrown");
	DReactionStep* locReactionStep = Get_ReactionStepResource();

	if(!locMCReactions.empty())
	{
		locReactionStep->Set_InitialParticleID(locMCReactions[0]->beam.PID());
		locReactionStep->Set_TargetParticleID(locMCReactions[0]->target.PID());
	}
	else //guess
	{
		locReactionStep->Set_InitialParticleID(Gamma);
		locReactionStep->Set_TargetParticleID(Proton);
	}

	for(size_t loc_i = 0; loc_i < locThrownSteps.size(); ++loc_i)
	{
		if(loc_i != 0) //else beam & target already set
		{
			locReactionStep = Get_ReactionStepResource();
			locReactionStep->Set_InitialParticleID(locThrownSteps[loc_i].first->PID());
			locReactionStep->Set_TargetParticleID(Unknown); //default (disabled)
		}
		for(size_t loc_j = 0; loc_j < locThrownSteps[loc_i].second.size(); ++loc_j)
			locReactionStep->Add_FinalParticleID(locThrownSteps[loc_i].second[loc_j]->PID());
		locReaction->Add_ReactionStep(locReactionStep);
	}

	return locReaction;
}

DReactionStep* DReaction_factory_Thrown::Get_ReactionStepResource(void)
{
	DReactionStep* locReactionStep;
	if(dReactionStepPool_Available.empty())
	{
		locReactionStep = new DReactionStep();
		dReactionStepPool_All.push_back(locReactionStep);
	}
	else
	{
		locReactionStep = dReactionStepPool_Available.back();
		locReactionStep->Reset();
		dReactionStepPool_Available.pop_back();
	}
	return locReactionStep;
}

void DReaction_factory_Thrown::Recycle_Reaction(DReaction* locReaction)
{
	//deletes reaction, but recycles steps
	for(size_t loc_i = 0; loc_i < locReaction->Get_NumReactionSteps(); ++loc_i)
	{
		DReactionStep* locReactionStep = const_cast<DReactionStep*>(locReaction->Get_ReactionStep(loc_i));
		dReactionStepPool_Available.push_back(locReactionStep);
	}
	delete locReaction;
}

//------------------
// EndRun
//------------------
void DReaction_factory_Thrown::EndRun()
{
}

//------------------
// Finish
//------------------
void DReaction_factory_Thrown::Finish()
{
	for(size_t loc_i = 0; loc_i < dReactionStepPool_All.size(); ++loc_i)
		delete dReactionStepPool_All[loc_i];
}


