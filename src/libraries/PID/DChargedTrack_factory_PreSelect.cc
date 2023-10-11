// $Id$
//
//    File: DChargedTrack_factory_PreSelect.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: staylor (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DChargedTrack_factory_PreSelect.h"

//------------------
// Init
//------------------
void DChargedTrack_factory_PreSelect::Init()
{
	// Setting this flag makes it so that JANA does not delete the objects in _data.  This factory will manage this memory.
	// This is because some/all of these pointers are just copied from earlier objects, and should not be deleted.
	SetFactoryFlag(NOT_OBJECT_OWNER);
	dResourcePool_ChargedTrack = new DResourcePool<DChargedTrack>();
	dResourcePool_ChargedTrack->Set_ControlParams(50, 20, 400, 4000, 0);

	dMinTrackingFOM = -1.0;
	dHasDetectorMatchFlag = true; //require tracks to have a detector match
}

//------------------
// BeginRun
//------------------
void DChargedTrack_factory_PreSelect::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	auto app = event->GetJApplication();
	app->SetDefaultParameter("PRESELECT:MIN_TRACKING_FOM", dMinTrackingFOM);
	app->SetDefaultParameter("PRESELECT:HAS_DETECTOR_MATCH_FLAG", dHasDetectorMatchFlag);
}

//------------------
// Process
//------------------
void DChargedTrack_factory_PreSelect::Process(const std::shared_ptr<const JEvent>& event)
{
	//Clear objects from last event
	dResourcePool_ChargedTrack->Recycle(dCreated);
	dCreated.clear();
	mData.clear();

	vector<const DChargedTrack*> locChargedTracks;
	event->Get(locChargedTracks);

	const DDetectorMatches* locDetectorMatches = NULL;
	event->GetSingle(locDetectorMatches);

	//cut on min-tracking-FOM and has-detector-match
	for(size_t loc_i = 0; loc_i < locChargedTracks.size(); ++loc_i)
	{
		DChargedTrack* locChargedTrack_PreSelected = new DChargedTrack(*locChargedTracks[loc_i]);
		
		// get rid of hypotheses which don't pass the preselect cuts
		auto locHypothesisItr = locChargedTrack_PreSelected->dChargedTrackHypotheses.begin();
		while( locHypothesisItr != locChargedTrack_PreSelected->dChargedTrackHypotheses.end()) {

			if(!Cut_TrackingFOM(*locHypothesisItr)) {
				locHypothesisItr = locChargedTrack_PreSelected->dChargedTrackHypotheses.erase(locHypothesisItr);  // delete and move to next
				continue;
			}
			if(!Cut_HasDetectorMatch(*locHypothesisItr, locDetectorMatches)) {
				locHypothesisItr = locChargedTrack_PreSelected->dChargedTrackHypotheses.erase(locHypothesisItr);  // delete and move to next
				continue;
			}
			
			locHypothesisItr++;   // move to next element
		}

		// keep the particle if any of the hypotheses survive
		if(locChargedTrack_PreSelected->dChargedTrackHypotheses.size() > 0)
			Insert(const_cast<DChargedTrack*>(locChargedTrack_PreSelected));
		else
			delete locChargedTrack_PreSelected;

	}

	dCreated = mData;
}

bool DChargedTrack_factory_PreSelect::Cut_HasDetectorMatch(const DChargedTrackHypothesis* locChargedTrackHypothesis, const DDetectorMatches* locDetectorMatches) const
{
	if(!dHasDetectorMatchFlag)
		return true;
	auto locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();
	return locDetectorMatches->Get_IsMatchedToHit(locTrackTimeBased);
}

bool DChargedTrack_factory_PreSelect::Cut_TrackingFOM(const DChargedTrackHypothesis* locChargedTrackHypothesis) const
{
	auto locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();
	double locFOM = TMath::Prob(locTrackTimeBased->chisq, locTrackTimeBased->Ndof);
	return ((locTrackTimeBased->Ndof == 0) ? true : (locFOM >= dMinTrackingFOM));
}

//------------------
// EndRun
//------------------
void DChargedTrack_factory_PreSelect::EndRun()
{
}

//------------------
// Finish
//------------------
void DChargedTrack_factory_PreSelect::Finish()
{
	for(auto locHypo : mData)
		Recycle_Hypothesis(locHypo);
	mData.clear();
	delete dResourcePool_ChargedTrack;
}

