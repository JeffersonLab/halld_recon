// $Id$
//
//    File: DEventRFBunch_factory_Calibrations.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DEventRFBunch_factory_Calibrations.h"

#include "DANA/DEvent.h"

//------------------
// Init
//------------------
void DEventRFBunch_factory_Calibrations::Init()
{
	dMinTrackingFOM = 0.001;
	dRFTDCSourceSystem = SYS_TOF;
	dMinHitRingsPerCDCSuperlayer = 2;
	dMinHitPlanesPerFDCPackage = 4;

	auto app = GetApplication();
	string locSystemName = "";
	app->SetDefaultParameter("RF_CALIB:SOURCE_SYSTEM", locSystemName);
	if(locSystemName != "")
		dRFTDCSourceSystem = NameToSystem(locSystemName.c_str());
}

//------------------
// BeginRun
//------------------
void DEventRFBunch_factory_Calibrations::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	DGeometry* locGeometry = DEvent::GetDGeometry(event);
	JCalibration* calibration = DEvent::GetJCalibration(event);

	vector<double> locBeamPeriodVector;
	calibration->Get("PHOTON_BEAM/RF/beam_period", locBeamPeriodVector);
	dBeamBunchPeriod = locBeamPeriodVector[0];

	double locTargetCenterZ;
	locGeometry->GetTargetZ(locTargetCenterZ);
	dTargetCenter.SetXYZ(0.0, 0.0, locTargetCenterZ);

	event->GetSingle(dParticleID);

	dCutAction_TrackHitPattern = new DCutAction_TrackHitPattern(NULL, dMinHitRingsPerCDCSuperlayer, dMinHitPlanesPerFDCPackage);
	dCutAction_TrackHitPattern->Initialize(event);
}

//------------------
// Process
//------------------
void DEventRFBunch_factory_Calibrations::Process(const std::shared_ptr<const JEvent>& event)
{
	//There should ALWAYS be one and only one DEventRFBunch created.
		//If there is not enough information, time is set to NaN

	//This factory is designed for calibrating the timing offsets between the tagger & SC.
	//It gets one RF time signal from the F1TDC system of choice, and tries to select the correct bunch using tracks hitting the SC.
	//It uses wire-based tracks instead of time-based tracks since the timing has not yet been calibrated.

	//Select Good Tracks
	vector<const DTrackWireBased*> locTrackWireBasedVector;
	Select_GoodTracks(event, locTrackWireBasedVector);

	//Get RF Time
	vector<const DRFTDCDigiTime*> locRFTDCDigiTimes;
	event->Get(locRFTDCDigiTimes);

	const DTTabUtilities* locTTabUtilities = NULL;
	event->GetSingle(locTTabUtilities);

	double locRFTime = numeric_limits<double>::quiet_NaN();
	bool locHitFoundFlag = false;
	for(size_t loc_i = 0; loc_i < locRFTDCDigiTimes.size(); ++loc_i)
	{
		const DRFTDCDigiTime* locRFTDCDigiTime = locRFTDCDigiTimes[loc_i];
		if(locRFTDCDigiTime->dSystem != dRFTDCSourceSystem)
			continue;

		if(locRFTDCDigiTime->dIsCAENTDCFlag)
			locRFTime = locTTabUtilities->Convert_DigiTimeToNs_CAEN1290TDC(locRFTDCDigiTime);
		else
			locRFTime = locTTabUtilities->Convert_DigiTimeToNs_F1TDC(locRFTDCDigiTime);

		locHitFoundFlag = true;
		break;
	}

	if(!locHitFoundFlag)
		Create_NaNRFBunch();
		return; // TODO: NWB: Verify

	//Select RF Bunch:
	Select_RFBunch(event, locTrackWireBasedVector, locRFTime);
	return; // TODO: Verify
}

void DEventRFBunch_factory_Calibrations::Select_GoodTracks(const std::shared_ptr<const JEvent>& event, vector<const DTrackWireBased*>& locSelectedWireBasedTracks) const
{
	//Select tracks:
		//For each particle (DTrackWireBased::candidateid), use the DTrackWireBased with the best tracking FOM
		//Only use DTrackWireBased's with tracking FOM > dMinTrackingFOM

	locSelectedWireBasedTracks.clear();

	vector<const DTrackWireBased*> locTrackWireBasedVector;
	event->Get(locTrackWireBasedVector);

	//select the best DTrackWireBased for each track: of tracks with good hit pattern, use best tracking FOM
	map<oid_t, const DTrackWireBased*> locBestTrackWireBasedMap; //lowest tracking chisq/ndf for each candidate id
	for(size_t loc_i = 0; loc_i < locTrackWireBasedVector.size(); ++loc_i)
	{
		if(locTrackWireBasedVector[loc_i]->FOM < dMinTrackingFOM)
			continue;
		if(!dCutAction_TrackHitPattern->Cut_TrackHitPattern(dParticleID, locTrackWireBasedVector[loc_i]))
			continue;
		oid_t locCandidateID = locTrackWireBasedVector[loc_i]->candidateid;
		if(locBestTrackWireBasedMap.find(locCandidateID) == locBestTrackWireBasedMap.end())
			locBestTrackWireBasedMap[locCandidateID] = locTrackWireBasedVector[loc_i];
		else if(locTrackWireBasedVector[loc_i]->FOM > (dynamic_cast<const DTrackWireBased*>(locBestTrackWireBasedMap[locCandidateID]))->FOM)
			locBestTrackWireBasedMap[locCandidateID] = locTrackWireBasedVector[loc_i];
	}

	//Save to the vector
	map<oid_t, const DTrackWireBased*>::iterator locIterator;
	for(locIterator = locBestTrackWireBasedMap.begin(); locIterator != locBestTrackWireBasedMap.end(); ++locIterator)
		locSelectedWireBasedTracks.push_back(locIterator->second);
}

jerror_t DEventRFBunch_factory_Calibrations::Select_RFBunch(const std::shared_ptr<const JEvent>& event, vector<const DTrackWireBased*>& locTrackWireBasedVector, double locRFTime)
{
	//If RF Time present:
	//Use tracks with matching SC hits, if any
	//If None: set DEventRFBunch::dTime to NaN

	vector<const DDetectorMatches*> locDetectorMatchVector;
	event->Get(locDetectorMatchVector, "WireBased");
	if(locDetectorMatchVector.empty())
	{
		cout << "WARNING: WIREBASED TRACKS NOT PRESENT IN DEventRFBunch_factory_Calibrations(). RETURNING NaN." << endl;
		return Create_NaNRFBunch();
	}
	const DDetectorMatches* locDetectorMatches = locDetectorMatchVector[0];

	vector<pair<double, const JObject*> > locTimes;
	int locBestRFBunchShift = 0, locHighestNumVotes = 0;

	//Use tracks with matching SC hits, if any
	if(Find_TrackTimes_SC(locDetectorMatches, locTrackWireBasedVector, locTimes))
		locBestRFBunchShift = Conduct_Vote(event, locRFTime, locTimes, locHighestNumVotes);
	else //SET NaN
		return Create_NaNRFBunch();

	DEventRFBunch* locEventRFBunch = new DEventRFBunch;
	locEventRFBunch->dTime = locRFTime + dBeamBunchPeriod*double(locBestRFBunchShift);
	locEventRFBunch->dTimeVariance = 0.0;
	locEventRFBunch->dNumParticleVotes = locHighestNumVotes;
	locEventRFBunch->dTimeSource = SYS_RF;
	Insert(locEventRFBunch);

	return NOERROR;
}

bool DEventRFBunch_factory_Calibrations::Find_TrackTimes_SC(const DDetectorMatches* locDetectorMatches, const vector<const DTrackWireBased*>& locTrackWireBasedVector, vector<pair<double, const JObject*> >& locTimes) const
{
	locTimes.clear();
	for(size_t loc_i = 0; loc_i < locTrackWireBasedVector.size(); ++loc_i)
	{
		const DTrackWireBased* locTrackWireBased = locTrackWireBasedVector[loc_i];

		shared_ptr<const DSCHitMatchParams> locSCHitMatchParams;
		if(!dParticleID->Get_BestSCMatchParams(locTrackWireBased, locDetectorMatches, locSCHitMatchParams))
			continue;

		double locPropagatedTime = locSCHitMatchParams->dHitTime - locSCHitMatchParams->dFlightTime + (dTargetCenter.Z() - locTrackWireBased->z())/29.9792458;
		locTimes.push_back(pair<double, const JObject*>(locPropagatedTime, locTrackWireBased));
	}

	return (!locTimes.empty());
}

int DEventRFBunch_factory_Calibrations::Conduct_Vote(const std::shared_ptr<const JEvent>& event, double locRFTime, vector<pair<double, const JObject*> >& locTimes, int& locHighestNumVotes)
{
	map<int, vector<const JObject*> > locNumBeamBucketsShiftedMap;
	set<int> locBestRFBunchShifts;

	locHighestNumVotes = Find_BestRFBunchShifts(locRFTime, locTimes, locNumBeamBucketsShiftedMap, locBestRFBunchShifts);
	if(locBestRFBunchShifts.size() == 1)
		return *locBestRFBunchShifts.begin();

	//break tie with total track hits (ndf), else break with total tracking chisq
	return Break_TieVote_Tracks(locNumBeamBucketsShiftedMap, locBestRFBunchShifts);
}

int DEventRFBunch_factory_Calibrations::Find_BestRFBunchShifts(double locRFHitTime, const vector<pair<double, const JObject*> >& locTimes, map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts)
{
	//then find the #beam buckets the RF time needs to shift to match it
	int locHighestNumVotes = 0;
	locNumBeamBucketsShiftedMap.clear();
	locBestRFBunchShifts.clear();

	for(unsigned int loc_i = 0; loc_i < locTimes.size(); ++loc_i)
	{
		double locDeltaT = locTimes[loc_i].first - locRFHitTime;
		int locNumBeamBucketsShifted = (locDeltaT > 0.0) ? int(locDeltaT/dBeamBunchPeriod + 0.5) : int(locDeltaT/dBeamBunchPeriod - 0.5);
		locNumBeamBucketsShiftedMap[locNumBeamBucketsShifted].push_back(locTimes[loc_i].second);

		int locNumVotesThisShift = locNumBeamBucketsShiftedMap[locNumBeamBucketsShifted].size();
		if(locNumVotesThisShift > locHighestNumVotes)
		{
			locHighestNumVotes = locNumVotesThisShift;
			locBestRFBunchShifts.clear();
			locBestRFBunchShifts.insert(locNumBeamBucketsShifted);
		}
		else if(locNumVotesThisShift == locHighestNumVotes)
			locBestRFBunchShifts.insert(locNumBeamBucketsShifted);
	}

	return locHighestNumVotes;
}

int DEventRFBunch_factory_Calibrations::Break_TieVote_Tracks(map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts)
{
	//Select the bunch with the most total track hits (highest total tracking NDF)
	//If still a tie: 
		//Select the bunch with the lowest total chisq

	int locBestRFBunchShift = 0;
	pair<int, double> locBestTrackingTotals(-1, 9.9E99); //ndf, chisq

	set<int>::const_iterator locSetIterator = locBestRFBunchShifts.begin();
	for(; locSetIterator != locBestRFBunchShifts.end(); ++locSetIterator)
	{
		int locRFBunchShift = *locSetIterator;
		int locTotalTrackingNDF = 0;
		double locTotalTrackingChiSq = 0.0;

		const vector<const JObject*>& locVoters = locNumBeamBucketsShiftedMap[locRFBunchShift];
		for(size_t loc_i = 0; loc_i < locVoters.size(); ++loc_i)
		{
			const DTrackWireBased* locTrackWireBased = dynamic_cast<const DTrackWireBased*>(locVoters[loc_i]);
			if(locTrackWireBased == NULL)
				continue;
			locTotalTrackingNDF += locTrackWireBased->Ndof;
			locTotalTrackingChiSq += locTrackWireBased->chisq;
		}

		if(locTotalTrackingNDF > locBestTrackingTotals.first)
		{
			locBestTrackingTotals = pair<int, double>(locTotalTrackingNDF, locTotalTrackingChiSq);
			locBestRFBunchShift = locRFBunchShift;
		}
		else if((locTotalTrackingNDF == locBestTrackingTotals.first) && (locTotalTrackingChiSq < locBestTrackingTotals.second))
		{
			locBestTrackingTotals = pair<int, double>(locTotalTrackingNDF, locTotalTrackingChiSq);
			locBestRFBunchShift = locRFBunchShift;
		}
	}

	return locBestRFBunchShift;
}

jerror_t DEventRFBunch_factory_Calibrations::Create_NaNRFBunch(void)
{
	DEventRFBunch* locEventRFBunch = new DEventRFBunch;
	locEventRFBunch->dTime = numeric_limits<double>::quiet_NaN();
	locEventRFBunch->dTimeVariance = numeric_limits<double>::quiet_NaN();
	locEventRFBunch->dNumParticleVotes = 0;
	locEventRFBunch->dTimeSource = SYS_NULL;
	Insert(locEventRFBunch);

	return NOERROR;
}

//------------------
// EndRun
//------------------
void DEventRFBunch_factory_Calibrations::EndRun()
{
}

//------------------
// Finish
//------------------
void DEventRFBunch_factory_Calibrations::Finish()
{
}

