#include "DChargedTrack_factory_Combo.h"

//------------------
// Init
//------------------
void DChargedTrack_factory_Combo::Init()
{
	auto app = GetApplication();
	dTrackSelectionTag = "PreSelect";
	app->SetDefaultParameter("COMBO:TRACK_SELECT_TAG", dTrackSelectionTag);
}

//------------------
// BeginRun
//------------------
void DChargedTrack_factory_Combo::BeginRun(const std::shared_ptr<const JEvent>& locEvent)
{
	vector<const DChargedTrackHypothesis*> locChargedTrackHypotheses;
	locEvent->Get(locChargedTrackHypotheses); //make sure that brun() is called for the default factory!!!   // TODO: Why wouldn't it be?
	dChargedTrackHypothesisFactory = dynamic_cast<DChargedTrackHypothesis_factory*>(locEvent->GetFactory<DChargedTrackHypothesis>());  // TODO: Bad!
	// This breaks factory swappability. What do you need this for, anyway? You can probably use JMetadata<DCharedTrackHypothesis> instead.
}

//------------------
// Process
//------------------
void DChargedTrack_factory_Combo::Process(const std::shared_ptr<const JEvent>& locEvent)
{
	dChargedTrackHypothesisFactory->Recycle_Hypotheses(dCreatedHypotheses);

	vector<const DChargedTrack*> locChargedTracks;
	locEvent->Get(locChargedTracks, dTrackSelectionTag.c_str());

	map<oid_t, const DChargedTrack*> locTrackToCandidateID;
	for(auto locChargedTrack : locChargedTracks)
		locTrackToCandidateID.emplace(locChargedTrack->candidateid, locChargedTrack);

	vector<const DTrackTimeBased*> locTimeBasedTracks;
	locEvent->Get(locTimeBasedTracks, "Combo");

	const DEventRFBunch* locEventRFBunch = nullptr;
	locEvent->GetSingle(locEventRFBunch);

	const DDetectorMatches* locDetectorMatches = nullptr;
	locEvent->GetSingle(locDetectorMatches, "Combo");

	//Sort new time-based tracks by charged track
	unordered_map<const DChargedTrack*, vector<const DTrackTimeBased*>> locTimeBasedByChargedTrack;
	for(auto& locTimeBasedTrack : locTimeBasedTracks)
	{
		auto locChargedTrack = locTrackToCandidateID[locTimeBasedTrack->candidateid];
		locTimeBasedByChargedTrack[locChargedTrack].push_back(locTimeBasedTrack);
	}

	//create and add new hypos
	for(auto& locChargedTrack : locChargedTracks)
	{
		auto locNewChargedTrack = new DChargedTrack(*locChargedTrack);
		for(auto& locTimeBasedTrack : locTimeBasedByChargedTrack[locChargedTrack])
		{
			//create new DChargedTrackHypothesis object
			auto locNewChargedTrackHypothesis = dChargedTrackHypothesisFactory->Create_ChargedTrackHypothesis(locEvent, locTimeBasedTrack, locDetectorMatches, locEventRFBunch);
			locNewChargedTrackHypothesis->AddAssociatedObject(locNewChargedTrack);
			dCreatedHypotheses.push_back(locNewChargedTrackHypothesis);
			locNewChargedTrack->dChargedTrackHypotheses.push_back(locNewChargedTrackHypothesis);
		}
		locNewChargedTrack->AddAssociatedObject(locChargedTrack);
		Insert(locNewChargedTrack);
	}
}
