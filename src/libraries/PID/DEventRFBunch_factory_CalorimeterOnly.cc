// $Id$
//
//    File: DEventRFBunch_factory_CalorimeterOnly.cc
//

#include "DEventRFBunch_factory_CalorimeterOnly.h"
#include "BCAL/DBCALShower.h"
#include "FCAL/DFCALShower.h"
#include "CCAL/DCCALShower.h"

#include "DANA/DEvent.h"


//------------------
// Init
//------------------
void DEventRFBunch_factory_CalorimeterOnly::Init()
{
	dMinTrackingFOM = 0.0;
	
	USE_BCAL = false;  
	USE_CCAL = true; 
	USE_FCAL = true;

	auto app = GetApplication();
	
	app->SetDefaultParameter("EVENTRFBUNCH_CAL:USE_BCAL_SHOWERS", USE_BCAL, "Use BCAL showers for calorimeter-only RF bunch calculation");
	app->SetDefaultParameter("EVENTRFBUNCH_CAL:USE_CCAL_SHOWERS", USE_CCAL, "Use CCAL showers for calorimeter-only RF bunch calculation");
	app->SetDefaultParameter("EVENTRFBUNCH_CAL:USE_FCAL_SHOWERS", USE_FCAL, "Use FCAL showers for calorimeter-only RF bunch calculation");
}

//------------------
// BeginRun
//------------------
void DEventRFBunch_factory_CalorimeterOnly::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	DGeometry* locGeometry = DEvent::GetDGeometry(event);
	JCalibration* locCalib = DEvent::GetJCalibration(event);

	vector<double> locBeamPeriodVector;
	locCalib->Get("PHOTON_BEAM/RF/beam_period", locBeamPeriodVector);
	dBeamBunchPeriod = locBeamPeriodVector[0];

	double locTargetCenterZ;
	locGeometry->GetTargetZ(locTargetCenterZ);
	dTargetCenter.SetXYZ(0.0, 0.0, locTargetCenterZ);

	event->GetSingle(dParticleID);
}

//------------------
// Process
//------------------
void DEventRFBunch_factory_CalorimeterOnly::Process(const std::shared_ptr<const JEvent>& event)
{
	//There should ALWAYS be one and only one DEventRFBunch created.
	//If there is not enough information, time is set to NaN

	//Select RF Bunch:
	vector<const DRFTime*> locRFTimes;
	event->Get(locRFTimes);
	if(!locRFTimes.empty())
		Select_RFBunch(event, locRFTimes[0]);
	else
		Create_NaNRFBunch();   // there should always be RFTime data, otherwise there's not enough info to choose
}

jerror_t DEventRFBunch_factory_CalorimeterOnly::Select_RFBunch(const std::shared_ptr<const JEvent>& event, const DRFTime* locRFTime)
{
	//If RF Time present:
		// Let neutral showers vote (assume PID = photon) on RF bunch
		//If None: set DEventRFBunch::dTime to NaN

	//Voting when RF time present:
		//Propagate track/shower times to vertex
		//Compare times to possible RF bunches, select the bunch with the most votes
		//If there is a tie: let DBeamPhoton's vote to break tie
			//If a tie, and voting with neutral showers:
				//Select the bunch with the highest total shower energy

	const DDetectorMatches* locDetectorMatches = NULL;
	event->GetSingle(locDetectorMatches);

	vector<pair<double, const JObject*> > locTimes;
	int locBestRFBunchShift = 0, locHighestNumVotes = 0;

	if(Find_NeutralTimes(event, locTimes))
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

int DEventRFBunch_factory_CalorimeterOnly::Conduct_Vote(const std::shared_ptr<const JEvent>& event, double locRFTime, vector<pair<double, const JObject*> >& locTimes, bool locUsedTracksFlag, int& locHighestNumVotes)
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

	//break tie with highest total shower energy
	return Break_TieVote_Neutrals(locNumBeamBucketsShiftedMap, locBestRFBunchShifts);
	
}


bool DEventRFBunch_factory_CalorimeterOnly::Find_NeutralTimes(const std::shared_ptr<const JEvent>& event, vector<pair<double, const JObject*> >& locTimes)
{
	locTimes.clear();

	if(USE_FCAL) {
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
	}

	if(USE_BCAL) {
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
	}
	
	if(USE_CCAL) {
		vector< const DCCALShower* > ccalShowers;
		event->Get( ccalShowers );
		
		for( size_t i = 0; i < ccalShowers.size(); ++i ){
		  DVector3 locHitPoint(ccalShowers[i]->x, ccalShowers[i]->y, ccalShowers[i]->z);
		  DVector3 locPath = locHitPoint - dTargetCenter;
		  double locPathLength = locPath.Mag();
		  if(!(locPathLength > 0.0))
			continue;
  
		  double locFlightTime = locPathLength/29.9792458;
		  double locHitTime = ccalShowers[i]->time;
		  locTimes.push_back( pair< double, const JObject*>( locHitTime - locFlightTime, ccalShowers[i] ) );
		}
	}
	
	    
	return (locTimes.size() > 1);
}

int DEventRFBunch_factory_CalorimeterOnly::Find_BestRFBunchShifts(double locRFHitTime, const vector<pair<double, const JObject*> >& locTimes, map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts)
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

bool DEventRFBunch_factory_CalorimeterOnly::Break_TieVote_BeamPhotons(vector<const DBeamPhoton*>& locBeamPhotons, double locRFTime, map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts, int locHighestNumVotes)
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


int DEventRFBunch_factory_CalorimeterOnly::Break_TieVote_Neutrals(map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts)
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

jerror_t DEventRFBunch_factory_CalorimeterOnly::Create_NaNRFBunch(void)
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
void DEventRFBunch_factory_CalorimeterOnly::EndRun()
{
}

//------------------
// Finish
//------------------
void DEventRFBunch_factory_CalorimeterOnly::Finish()
{
}

