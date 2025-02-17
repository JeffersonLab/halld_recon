// $Id$
//
//    File: DEventRFBunch_factory.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DEventRFBunch_factory.h"
#include "BCAL/DBCALShower.h"
#include "FCAL/DFCALShower.h"
#include "CCAL/DCCALShower.h"
#include "DANA/DEvent.h"
#include "DANA/DObjectID.h"


//------------------
// Init
//------------------
void DEventRFBunch_factory::Init()
{
	dMinTrackingFOM = 0.0;
	OVERRIDE_TAG = "";

	auto app = GetApplication();
	app->SetDefaultParameter("EVENTRFBUNCH:USE_TAG", OVERRIDE_TAG, "Use a particular tag for the general RF bunch calculation instead of the default calculation.");
}

//------------------
// BeginRun
//------------------
void DEventRFBunch_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
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
}

//------------------
// Process
//------------------
void DEventRFBunch_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	//There should ALWAYS be one and only one DEventRFBunch created.
	//If there is not enough information, time is set to NaN

	if(OVERRIDE_TAG != "") {
		vector<const DEventRFBunch *> rfBunchVec;
		event->Get(rfBunchVec, OVERRIDE_TAG.c_str());
		
		if(rfBunchVec.size() > 0) {
			DEventRFBunch *rfBunchCopy = new DEventRFBunch(*rfBunchVec[0]);
			Insert(rfBunchCopy);
			return;
		} else {
			jerr << "Could not find DEventRFBunch objects with tag " << OVERRIDE_TAG
				 << ", falling back to default calculation ..." << endl;
		}
	}


	//Select Good Tracks
	vector<const DTrackTimeBased*> locTrackTimeBasedVector;
	Select_GoodTracks(event, locTrackTimeBasedVector);

	//Select RF Bunch:
	vector<const DRFTime*> locRFTimes;
	event->Get(locRFTimes);
	if(!locRFTimes.empty())
		Select_RFBunch(event, locTrackTimeBasedVector, locRFTimes[0]);
	else
		Select_RFBunch_NoRFTime(event, locTrackTimeBasedVector);
	// TODO: Verify return value thing
}

void DEventRFBunch_factory::Select_GoodTracks(const std::shared_ptr<const JEvent>& event, vector<const DTrackTimeBased*>& locSelectedTimeBasedTracks) const
{
	//Select tracks:
  //For each particle (DTrackTimeBased::candidateid), use the DTrackTimeBased with the best tracking FOM
		//Only use DTrackTimeBased's with tracking FOM > dMinTrackingFOM

	locSelectedTimeBasedTracks.clear();

	vector<const DTrackTimeBased*> locTrackTimeBasedVector;
	event->Get(locTrackTimeBasedVector);

	//select the best DTrackTimeBased for each track: use best tracking FOM
	map<oid_t, const DTrackTimeBased*> locBestTrackTimeBasedMap; //lowest tracking chisq/ndf for each candidate id
	for(size_t loc_i = 0; loc_i < locTrackTimeBasedVector.size(); ++loc_i)
	{
		oid_t locCandidateID = locTrackTimeBasedVector[loc_i]->candidateid;
		if(locBestTrackTimeBasedMap.find(locCandidateID) == locBestTrackTimeBasedMap.end())
			locBestTrackTimeBasedMap[locCandidateID] = locTrackTimeBasedVector[loc_i];
		else if(locTrackTimeBasedVector[loc_i]->FOM > locBestTrackTimeBasedMap[locCandidateID]->FOM)
			locBestTrackTimeBasedMap[locCandidateID] = locTrackTimeBasedVector[loc_i];
	}

	//Select tracks with good tracking FOM
	map<oid_t, const DTrackTimeBased*>::iterator locIterator;
	for(locIterator = locBestTrackTimeBasedMap.begin(); locIterator != locBestTrackTimeBasedMap.end(); ++locIterator)
	{
		const DTrackTimeBased* locTrackTimeBased = locIterator->second;
		if(locTrackTimeBased->FOM >= dMinTrackingFOM)
			locSelectedTimeBasedTracks.push_back(locTrackTimeBased);
	}
}

jerror_t DEventRFBunch_factory::Select_RFBunch(const std::shared_ptr<const JEvent>& event, vector<const DTrackTimeBased*>& locTrackTimeBasedVector, const DRFTime* locRFTime)
{
  //If RF Time present:
  // Use track matches in the following order: SC, TOF, BCAL, FCAL. 
  // If none: Let neutral showers vote (assume PID = photon) on RF bunch
  // If None: set DEventRFBunch::dTime to NaN

  //Voting when RF time present:
  //Propagate track/shower times to vertex
  //Compare times to possible RF bunches, select the bunch with the most votes
  //If there is a tie: let DBeamPhoton's vote to break tie
  //If still a tie, and voting with tracks:
  //Select the bunch with the most total track hits (highest total tracking NDF)
  //If still a tie: 
  //Select the bunch with the lowest total chisq
  //If still a tie, and voting with neutral showers:
  //Select the bunch with the highest total shower energy

	const DDetectorMatches* locDetectorMatches = NULL;
	event->GetSingle(locDetectorMatches);

	vector<pair<double, const JObject*> > locTimes;
	int locBestRFBunchShift = 0, locHighestNumVotes = 0;

	if(Find_TrackTimes_All(locDetectorMatches, locTrackTimeBasedVector, locTimes))
		locBestRFBunchShift = Conduct_Vote(event, locRFTime->dTime, locTimes, true, locHighestNumVotes);
	else if(Find_NeutralTimes(event, locTimes))
		locBestRFBunchShift = Conduct_Vote(event, locRFTime->dTime, locTimes, false, locHighestNumVotes);
	else //PASS-THROUGH, WILL HAVE TO RESOLVE WHEN COMBOING
		locBestRFBunchShift = 0;

	DEventRFBunch* locEventRFBunch = new DEventRFBunch();
	locEventRFBunch->dTime = locRFTime->dTime + dBeamBunchPeriod*double(locBestRFBunchShift);
	locEventRFBunch->dTimeVariance = locRFTime->dTimeVariance;
	locEventRFBunch->dNumParticleVotes = locHighestNumVotes;
	locEventRFBunch->dTimeSource = SYS_RF;
	Insert(locEventRFBunch);

	return NOERROR;
}

int DEventRFBunch_factory::Conduct_Vote(const std::shared_ptr<const JEvent>& event, double locRFTime, vector<pair<double, const JObject*> >& locTimes, bool locUsedTracksFlag, int& locHighestNumVotes)
{
	map<int, vector<const JObject*> > locNumBeamBucketsShiftedMap;
	set<int> locBestRFBunchShifts;

	locHighestNumVotes = Find_BestRFBunchShifts(locRFTime, locTimes, locNumBeamBucketsShiftedMap, locBestRFBunchShifts);
	if(locBestRFBunchShifts.size() == 1)
		return *locBestRFBunchShifts.begin();

	//tied: break with beam photons
	vector<const DBeamPhoton*> locBeamPhotons;
	event->Get(locBeamPhotons);
	if(Break_TieVote_BeamPhotons(locBeamPhotons, locRFTime, locNumBeamBucketsShiftedMap, locBestRFBunchShifts, locHighestNumVotes))
		return *locBestRFBunchShifts.begin();

	//still tied
	if(locUsedTracksFlag)
	{
		//break tie with total track hits (ndf), else break with total tracking chisq
		return Break_TieVote_Tracks(locNumBeamBucketsShiftedMap, locBestRFBunchShifts);
	}
	else //neutrals
	{
		//break tie with highest total shower energy
		return Break_TieVote_Neutrals(locNumBeamBucketsShiftedMap, locBestRFBunchShifts);
	}
}

bool DEventRFBunch_factory::Find_TrackTimes_SCTOF(const DDetectorMatches* locDetectorMatches, const vector<const DTrackTimeBased*>& locTrackTimeBasedVector, vector<pair<double, const JObject*> >& locTimes) const
{
	locTimes.clear();
	for(size_t loc_i = 0; loc_i < locTrackTimeBasedVector.size(); ++loc_i)
	{
		const DTrackTimeBased* locTrackTimeBased = locTrackTimeBasedVector[loc_i];
		double locP = locTrackTimeBased->momentum().Mag();

		//TOF resolution = 80ps, 3sigma = 240ps
			//max PID delta-t = 762ps (assuming 499 MHz and no buffer)
			//for pion-proton: delta-t is ~750ps at ~170 MeV/c or lower: cannot have proton this slow anyway
			//for pion-kaon delta-t is ~750ps at ~80 MeV/c or lower: won't reconstruct these anyway, and not likely to be forward-going anyway
		shared_ptr<const DTOFHitMatchParams> locTOFHitMatchParams;
		if((locP > 0.5) && dParticleID->Get_BestTOFMatchParams(locTrackTimeBased, locDetectorMatches, locTOFHitMatchParams))
		{
			double locPropagatedTime = locTOFHitMatchParams->dHitTime - locTOFHitMatchParams->dFlightTime + (dTargetCenter.Z() - locTrackTimeBased->z())/29.9792458;
			locTimes.push_back(pair<double, const JObject*>(locPropagatedTime, locTrackTimeBased));
			continue;
		}

		//Prefer TOF over SC in nose region because hard to tell which SC counter should have been hit
		shared_ptr<const DSCHitMatchParams> locSCHitMatchParams;
		if(!dParticleID->Get_BestSCMatchParams(locTrackTimeBased, locDetectorMatches, locSCHitMatchParams))
			continue;

		double locPropagatedTime = locSCHitMatchParams->dHitTime - locSCHitMatchParams->dFlightTime + (dTargetCenter.Z() - locTrackTimeBased->z())/29.9792458;
		locTimes.push_back(pair<double, const JObject*>(locPropagatedTime, locTrackTimeBased));
	}

	return (!locTimes.empty());
}

bool DEventRFBunch_factory::Find_TrackTimes_All(const DDetectorMatches* locDetectorMatches, const vector<const DTrackTimeBased*>& locTrackTimeBasedVector, vector<pair<double, const JObject*> >& locTimes)
{
	locTimes.clear();

	//Use TOF/BCAL time to match to RF bunches:
		//FCAL time resolution not good enough (right?)
		//max can be off = rf-frequency/2 ns (if off by more, will pick wrong beam bunch)
	for(size_t loc_i = 0; loc_i < locTrackTimeBasedVector.size(); ++loc_i)
	{
	  const DTrackTimeBased* locTrackTimeBased = locTrackTimeBasedVector[loc_i];

	  // start with SC: closest to target, minimal dependence on particle type
	  shared_ptr<const DSCHitMatchParams> locSCHitMatchParams;
	  if(dParticleID->Get_BestSCMatchParams(locTrackTimeBased, locDetectorMatches, locSCHitMatchParams))
	    {
	      double locPropagatedTime = locSCHitMatchParams->dHitTime - locSCHitMatchParams->dFlightTime + (dTargetCenter.Z() - locTrackTimeBased->z())/29.9792458;
	      locTimes.push_back(pair<double, const JObject*>(locPropagatedTime, locTrackTimeBased));
	      continue;
	      
	    }
	  
	  // Next use the TOF (best timing resolution for outer detectors
	  //TOF resolution = 80ps, 3sigma = 240ps
	  //max PID delta-t = 762ps (assuming 499 MHz and no buffer)
	  //for pion-proton: delta-t is ~750ps at ~170 MeV/c or lower: cannot have proton this slow anyway
	  //for pion-kaon delta-t is ~750ps at ~80 MeV/c or lower: won't reconstruct these anyway, and not likely to be forward-going anyway
	  shared_ptr<const DTOFHitMatchParams> locTOFHitMatchParams;
	  if(dParticleID->Get_BestTOFMatchParams(locTrackTimeBased, locDetectorMatches, locTOFHitMatchParams))
	    {
	      double locPropagatedTime = locTOFHitMatchParams->dHitTime - locTOFHitMatchParams->dFlightTime + (dTargetCenter.Z() - locTrackTimeBased->z())/29.9792458;
	      locTimes.push_back(pair<double, const JObject*>(locPropagatedTime, locTrackTimeBased));
	      continue;
	    }
	  
	  // Else match to BCAL if fast enough (low time resolution for slow particles)
	  double locP = locTrackTimeBased->momentum().Mag();
	  //at 250 MeV/c, BCAL t-resolution is ~310ps (3sigma = 920ps), BCAL delta-t error is ~40ps: ~960 ps < 1 ns: OK!!
	  //at 225 MeV/c, BCAL t-resolution is ~333ps (3sigma = 999ps), BCAL delta-t error is ~40ps: ~1040ps: bad at 499 MHz
	  if((locP < 0.25) && (dBeamBunchPeriod < 2.005))
	    continue; //too slow for the BCAL to distinguish RF bunch
	  
	  shared_ptr<const DBCALShowerMatchParams> locBCALShowerMatchParams;
	  if(dParticleID->Get_BestBCALMatchParams(locTrackTimeBased, locDetectorMatches, locBCALShowerMatchParams))
	    {
	      const DBCALShower* locBCALShower = locBCALShowerMatchParams->dBCALShower;
	      double locPropagatedTime = locBCALShower->t - locBCALShowerMatchParams->dFlightTime + (dTargetCenter.Z() - locTrackTimeBased->z())/29.9792458;
	      locTimes.push_back(pair<double, const JObject*>(locPropagatedTime, locTrackTimeBased));
	      continue;
	    }

	  // If all else fails, use FCAL 
	  shared_ptr<const DFCALShowerMatchParams> locFCALShowerMatchParams;
	  if(dParticleID->Get_BestFCALMatchParams(locTrackTimeBased, locDetectorMatches, locFCALShowerMatchParams))
	    {
	      const DFCALShower* locFCALShower = locFCALShowerMatchParams->dFCALShower;
	      double locPropagatedTime = locFCALShower->getTime() - locFCALShowerMatchParams->dFlightTime + (dTargetCenter.Z() - locTrackTimeBased->z())/29.9792458;
	      locTimes.push_back(pair<double, const JObject*>(locPropagatedTime, locTrackTimeBased));
	      continue;
	    }

	  
	}

	return (!locTimes.empty());
}

bool DEventRFBunch_factory::Find_NeutralTimes(const std::shared_ptr<const JEvent>& event, vector<pair<double, const JObject*> >& locTimes)
{
	locTimes.clear();

	vector< const DFCALShower* > fcalShowers;
	event->Get( fcalShowers );

	for( size_t i = 0; i < fcalShowers.size(); ++i ){

	  DVector3 locHitPoint = fcalShowers[i]->getPosition();
	  DVector3 locPath = locHitPoint - dTargetCenter;
	  double locPathLength = locPath.Mag();
	  if(!(locPathLength > 0.0))
	    continue;
	  
	  double locFlightTime = locPathLength/29.9792458;
	  double locHitTime = fcalShowers[i]->getTime();
	  locTimes.push_back( pair< double, const JObject*>( locHitTime - locFlightTime, fcalShowers[i] ) );
	}

	vector< const DBCALShower* > bcalShowers;
	event->Get( bcalShowers );

	for( size_t i = 0; i < bcalShowers.size(); ++i ){

	  DVector3 locHitPoint( bcalShowers[i]->x, bcalShowers[i]->y, bcalShowers[i]->z );
	  DVector3 locPath = locHitPoint - dTargetCenter;
	  double locPathLength = locPath.Mag();
	  if(!(locPathLength > 0.0))
	    continue;
	  
	  double locFlightTime = locPathLength/29.9792458;
	  double locHitTime = bcalShowers[i]->t;
	  locTimes.push_back( pair< double, const JObject*>( locHitTime - locFlightTime, bcalShowers[i] ) );
	}
	/*
	vector< const DCCALShower* > ccalShowers;
	event->Get( ccalShowers );

	for( size_t i = 0; i < ccalShowers.size(); ++i ){

	  DVector3 locHitPoint( ccalShowers[i]->x, ccalShowers[i]->y, ccalShowers[i]->z );
	  DVector3 locPath = locHitPoint - dTargetCenter;
	  double locPathLength = locPath.Mag();
	  if(!(locPathLength > 0.0))
	    continue;
	  
	  double locFlightTime = locPathLength/29.9792458;
	  double locHitTime = ccalShowers[i]->time;
	  locTimes.push_back( pair< double, const JObject*>( locHitTime - locFlightTime, ccalShowers[i] ) );
	}
	*/  
	return (locTimes.size() > 1);
}

int DEventRFBunch_factory::Find_BestRFBunchShifts(double locRFHitTime, const vector<pair<double, const JObject*> >& locTimes, map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts)
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

bool DEventRFBunch_factory::Break_TieVote_BeamPhotons(vector<const DBeamPhoton*>& locBeamPhotons, double locRFTime, map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts, int locHighestNumVotes)
{
	//locHighestNumVotes intentionally passed-in as value-type (non-reference)
		//beam photons are only used to BREAK the tie, not count as equal votes

	set<int> locInputRFBunchShifts = locBestRFBunchShifts; //only test these RF bunch selections
	for(size_t loc_i = 0; loc_i < locBeamPhotons.size(); ++loc_i)
	{
		double locDeltaT = locBeamPhotons[loc_i]->time() - locRFTime;
		int locNumBeamBucketsShifted = (locDeltaT > 0.0) ? int(locDeltaT/dBeamBunchPeriod + 0.5) : int(locDeltaT/dBeamBunchPeriod - 0.5);
		if(locInputRFBunchShifts.find(locNumBeamBucketsShifted) == locInputRFBunchShifts.end())
			continue; //only use beam votes to break input tie, not contribute to other beam buckets

		locNumBeamBucketsShiftedMap[locNumBeamBucketsShifted].push_back(locBeamPhotons[loc_i]);

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

	return (locBestRFBunchShifts.size() == 1);
}

int DEventRFBunch_factory::Break_TieVote_Tracks(map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts)
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
			const DTrackTimeBased* locTrackTimeBased = dynamic_cast<const DTrackTimeBased*>(locVoters[loc_i]);
			if(locTrackTimeBased == NULL)
				continue;
			locTotalTrackingNDF += locTrackTimeBased->Ndof;
			locTotalTrackingChiSq += locTrackTimeBased->chisq;
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

int DEventRFBunch_factory::Break_TieVote_Neutrals(map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts)
{
  //Break tie with highest total shower energy

  int locBestRFBunchShift = 0;
  double locHighestTotalEnergy = 0.0;

  set<int>::const_iterator locSetIterator = locBestRFBunchShifts.begin();
  for(; locSetIterator != locBestRFBunchShifts.end(); ++locSetIterator)
    {
      int locRFBunchShift = *locSetIterator;
      double locTotalEnergy = 0.0;

      const vector<const JObject*>& locVoters = locNumBeamBucketsShiftedMap[locRFBunchShift];
      for(size_t loc_i = 0; loc_i < locVoters.size(); ++loc_i)
	{
	  // the pointers in locVoters that we care about will either be those to DBCALShower
	  // or DFCALShower objects -- figure out which and record the energy
	  
	  const DFCALShower* fcalShower = dynamic_cast< const DFCALShower* >( locVoters[loc_i] );
	  
	  if( fcalShower != NULL ){

	    locTotalEnergy += fcalShower->getEnergy();
	  }
	  else{
	    
	    const DBCALShower* bcalShower = dynamic_cast< const DBCALShower* >( locVoters[loc_i] );
	    if( bcalShower != NULL ){
	      
	      locTotalEnergy += bcalShower->E;
	    }
	    else{
	      const DCCALShower* ccalShower = dynamic_cast< const DCCALShower* >( locVoters[loc_i] );
	      if( ccalShower != NULL ){
		
		locTotalEnergy += ccalShower->E;
	      }
	    }
	  }
	}
      
      if(locTotalEnergy > locHighestTotalEnergy)
	{
	  locHighestTotalEnergy = locTotalEnergy;
	  locBestRFBunchShift = locRFBunchShift;
	}
    }
  
  return locBestRFBunchShift;
}


jerror_t DEventRFBunch_factory::Select_RFBunch_NoRFTime(const std::shared_ptr<const JEvent>& event, vector<const DTrackTimeBased*>& locTrackTimeBasedVector)
{
	//If no RF time:
		//Use SC hits that have a track match to compute RF time guess, if any
			//Times could be modulo the rf-frequency still: line them up (after propagating to beamline)
			//Use RF time guess to vote, just as in found-rf-time case
		//If none:
			//Set RF time as NaN

	const DDetectorMatches* locDetectorMatches = NULL;
	event->GetSingle(locDetectorMatches);

	vector<pair<double, const JObject*> > locTimes;
	if(!Find_TrackTimes_SCTOF(locDetectorMatches, locTrackTimeBasedVector, locTimes))
		return Create_NaNRFBunch();
	DetectorSystem_t locTimeSource = SYS_START;

	double locRFTimeGuess, locTimeVariance;
	Get_RFTimeGuess(locTimes, locRFTimeGuess, locTimeVariance);

	//OK, now have RF time guess: vote
	int locHighestNumVotes = 0;
	int locBestRFBunchShift = Conduct_Vote(event, locRFTimeGuess, locTimes, true, locHighestNumVotes);

	DEventRFBunch *locEventRFBunch = new DEventRFBunch;
	locEventRFBunch->dTime = locRFTimeGuess + dBeamBunchPeriod*double(locBestRFBunchShift);
	locEventRFBunch->dTimeVariance = locTimeVariance;
	locEventRFBunch->dNumParticleVotes = locHighestNumVotes;
	locEventRFBunch->dTimeSource = locTimeSource;
	Insert(locEventRFBunch);

	return NOERROR;
}

bool DEventRFBunch_factory::Get_RFTimeGuess(const std::shared_ptr<const JEvent>& event, double& locRFTimeGuess, double& locRFVariance, DetectorSystem_t& locTimeSource) const
{
	//Meant to be called externally

	//Only call if no RF time:
		//Use SC hits that have a track match to compute RF time guess, if any
			//Times could be modulo the rf-frequency still: line them up

	vector<const DTrackTimeBased*> locTrackTimeBasedVector;
	Select_GoodTracks(event, locTrackTimeBasedVector);

	const DDetectorMatches* locDetectorMatches = NULL;
	event->GetSingle(locDetectorMatches);

	locTimeSource = SYS_NULL;
	vector<pair<double, const JObject*> > locTimes;
	if(!Find_TrackTimes_SCTOF(locDetectorMatches, locTrackTimeBasedVector, locTimes))
		return false;
	locTimeSource = SYS_START;

	Get_RFTimeGuess(locTimes, locRFTimeGuess, locRFVariance);
	return true;
}

void DEventRFBunch_factory::Get_RFTimeGuess(vector<pair<double, const JObject*> >& locTimes, double& locRFTimeGuess, double& locRFVariance) const
{
	//Only call if no RF time:
		//Use SC hits that have a track match to compute RF time guess, if any
			//Times could be modulo the rf-frequency still: line them up

	locRFTimeGuess = locTimes[0].first;
	for(size_t loc_i = 1; loc_i < locTimes.size(); ++loc_i)
	{
		double locDeltaT = locTimes[loc_i].first - locTimes[0].first;
		double locNumBeamBucketsShifted = (locDeltaT > 0.0) ? floor(locDeltaT/dBeamBunchPeriod + 0.5) : floor(locDeltaT/dBeamBunchPeriod - 0.5);
		locRFTimeGuess += locTimes[loc_i].first - locNumBeamBucketsShifted*dBeamBunchPeriod;
	}
	locRFTimeGuess /= double(locTimes.size());

	locRFVariance = 0.3*0.3/double(locTimes.size()); //Un-hard-code SC time resolution!!
}

jerror_t DEventRFBunch_factory::Create_NaNRFBunch(void)
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
void DEventRFBunch_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DEventRFBunch_factory::Finish()
{
}

