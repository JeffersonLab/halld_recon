// $Id$
//
//		File: DDetectorMatches_factory_WireBased.cc
// Created: Tue Aug	9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DDetectorMatches_factory_WireBased.h"

//------------------
// Init
//------------------
void DDetectorMatches_factory_WireBased::Init()
{
   ENABLE_FCAL_SINGLE_HITS = false;
   GetApplication()->SetDefaultParameter("PID:ENABLE_FCAL_SINGLE_HITS",ENABLE_FCAL_SINGLE_HITS);
}

//------------------
// BeginRun
//------------------
void DDetectorMatches_factory_WireBased::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	//LEAVE THIS EMPTY!!! OR ELSE WON'T BE INITIALIZED PROPERLY WHEN "COMBO" FACTORY CALLS Create_DDetectorMatches ON REST DATA!!
}

//------------------
// Process
//------------------
void DDetectorMatches_factory_WireBased::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DTrackWireBased*> locTrackWireBasedVector;
	event->Get(locTrackWireBasedVector);

	DDetectorMatches* locDetectorMatches = Create_DDetectorMatches(event, locTrackWireBasedVector);
	Insert(locDetectorMatches);
}

DDetectorMatches* DDetectorMatches_factory_WireBased::Create_DDetectorMatches(const std::shared_ptr<const JEvent>& event, vector<const DTrackWireBased*>& locTrackWireBasedVector)
{
	const DParticleID* locParticleID = NULL;
	event->GetSingle(locParticleID);

	vector<const DSCHit*> locSCHits;
	event->Get(locSCHits);

	vector<const DTOFPoint*> locTOFPoints;
	event->Get(locTOFPoints);

	vector<const DFCALShower*> locFCALShowers;
	event->Get(locFCALShowers);

	vector<const DBCALShower*> locBCALShowers;
	event->Get(locBCALShowers);

	DDetectorMatches* locDetectorMatches = new DDetectorMatches();

	//Match tracks to showers/hits
	for(size_t loc_i = 0; loc_i < locTrackWireBasedVector.size(); ++loc_i)
	{
		MatchToBCAL(locParticleID, locTrackWireBasedVector[loc_i], locBCALShowers, locDetectorMatches);
		MatchToTOF(locParticleID, locTrackWireBasedVector[loc_i], locTOFPoints, locDetectorMatches);
		MatchToFCAL(locParticleID, locTrackWireBasedVector[loc_i], locFCALShowers, locDetectorMatches);
		MatchToSC(locParticleID, locTrackWireBasedVector[loc_i], locSCHits, locDetectorMatches);
	}

	//Find nearest tracks to showers
	for(size_t loc_i = 0; loc_i < locBCALShowers.size(); ++loc_i)
		MatchToTrack(locParticleID, locBCALShowers[loc_i], locTrackWireBasedVector, locDetectorMatches);
	for(size_t loc_i = 0; loc_i < locFCALShowers.size(); ++loc_i)
		MatchToTrack(locParticleID, locFCALShowers[loc_i], locTrackWireBasedVector, locDetectorMatches);

	// Try to find matches between tracks and single hits in FCAL
	if (ENABLE_FCAL_SINGLE_HITS){
	  vector<const DFCALHit*> locFCALHits;
	  event->Get(locFCALHits);
	  if (locFCALHits.size()>0){
	    vector<const DFCALHit*>locSingleHits;
	    locParticleID->GetSingleFCALHits(locFCALShowers,locFCALHits,
					     locSingleHits);
	    
	    for (size_t loc_j=0;loc_j<locTrackWireBasedVector.size();loc_j++){
	      MatchToFCAL(locParticleID,locTrackWireBasedVector[loc_j],
			  locSingleHits,locDetectorMatches);
	    }	  
	  }
	}

	//Set flight-time/p correlations
	for(size_t loc_i = 0; loc_i < locTrackWireBasedVector.size(); ++loc_i)
	{
		double locFlightTimePCorrelation = locParticleID->Calc_BCALFlightTimePCorrelation(locTrackWireBasedVector[loc_i], locDetectorMatches);
		if(isfinite(locFlightTimePCorrelation))
			locDetectorMatches->Set_FlightTimePCorrelation(locTrackWireBasedVector[loc_i], SYS_BCAL, locFlightTimePCorrelation);

		locFlightTimePCorrelation = locParticleID->Calc_TOFFlightTimePCorrelation(locTrackWireBasedVector[loc_i], locDetectorMatches);
		if(isfinite(locFlightTimePCorrelation))
			locDetectorMatches->Set_FlightTimePCorrelation(locTrackWireBasedVector[loc_i], SYS_TOF, locFlightTimePCorrelation);

		locFlightTimePCorrelation = locParticleID->Calc_FCALFlightTimePCorrelation(locTrackWireBasedVector[loc_i], locDetectorMatches);
		if(isfinite(locFlightTimePCorrelation))
			locDetectorMatches->Set_FlightTimePCorrelation(locTrackWireBasedVector[loc_i], SYS_FCAL, locFlightTimePCorrelation);

		locFlightTimePCorrelation = locParticleID->Calc_SCFlightTimePCorrelation(locTrackWireBasedVector[loc_i], locDetectorMatches);
		if(isfinite(locFlightTimePCorrelation))
			locDetectorMatches->Set_FlightTimePCorrelation(locTrackWireBasedVector[loc_i], SYS_START, locFlightTimePCorrelation);
	}

	return locDetectorMatches;
}

void DDetectorMatches_factory_WireBased::MatchToBCAL(const DParticleID* locParticleID, const DTrackWireBased* locTrackWireBased, const vector<const DBCALShower*>& locBCALShowers, DDetectorMatches* locDetectorMatches) const
{
  if (locTrackWireBased->extrapolations.at(SYS_BCAL).size()==0) return;

	double locInputStartTime = locTrackWireBased->t0();
	for(size_t loc_i = 0; loc_i < locBCALShowers.size(); ++loc_i)
	{
	  shared_ptr<DBCALShowerMatchParams>locShowerMatchParams;
	  if(locParticleID->Cut_MatchDistance(locTrackWireBased->extrapolations.at(SYS_BCAL), locBCALShowers[loc_i], locInputStartTime, locShowerMatchParams))
	    locDetectorMatches->Add_Match(locTrackWireBased, locBCALShowers[loc_i], locShowerMatchParams);
	}
}

void DDetectorMatches_factory_WireBased::MatchToTOF(const DParticleID* locParticleID, const DTrackWireBased* locTrackWireBased, const vector<const DTOFPoint*>& locTOFPoints, DDetectorMatches* locDetectorMatches) const
{
	double locInputStartTime = locTrackWireBased->t0();
	const vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackWireBased->extrapolations.at(SYS_TOF);
	for(size_t loc_i = 0; loc_i < locTOFPoints.size(); ++loc_i)
	{
	  shared_ptr<DTOFHitMatchParams>locTOFHitMatchParams;
	  if(locParticleID->Cut_MatchDistance(extrapolations, locTOFPoints[loc_i], locInputStartTime, locTOFHitMatchParams))
	    locDetectorMatches->Add_Match(locTrackWireBased, locTOFPoints[loc_i], locTOFHitMatchParams);
	}
}

void DDetectorMatches_factory_WireBased::MatchToFCAL(const DParticleID* locParticleID, const DTrackWireBased* locTrackWireBased, const vector<const DFCALShower*>& locFCALShowers, DDetectorMatches* locDetectorMatches) const
{
	double locInputStartTime = locTrackWireBased->t0();
	const vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackWireBased->extrapolations.at(SYS_FCAL);
	for(size_t loc_i = 0; loc_i < locFCALShowers.size(); ++loc_i)
	{
	  shared_ptr<DFCALShowerMatchParams> locShowerMatchParams;
	  if(locParticleID->Cut_MatchDistance(extrapolations, locFCALShowers[loc_i], locInputStartTime, locShowerMatchParams))
	    locDetectorMatches->Add_Match(locTrackWireBased, locFCALShowers[loc_i], locShowerMatchParams);
	}
}

void DDetectorMatches_factory_WireBased::MatchToSC(const DParticleID* locParticleID, const DTrackWireBased* locTrackWireBased, const vector<const DSCHit*>& locSCHits, DDetectorMatches* locDetectorMatches) const
{
	double locInputStartTime = locTrackWireBased->t0();
	const vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackWireBased->extrapolations.at(SYS_START);
	for(size_t loc_i = 0; loc_i < locSCHits.size(); ++loc_i)
	{
	  shared_ptr<DSCHitMatchParams> locSCHitMatchParams;
	  if(locParticleID->Cut_MatchDistance(extrapolations, locSCHits[loc_i], locInputStartTime, locSCHitMatchParams, true))
	    locDetectorMatches->Add_Match(locTrackWireBased, locSCHits[loc_i], locSCHitMatchParams);
	}
}

void DDetectorMatches_factory_WireBased::MatchToTrack(const DParticleID* locParticleID, const DBCALShower* locBCALShower, const vector<const DTrackWireBased*>& locTrackWireBasedVector, DDetectorMatches* locDetectorMatches) const
{
	double locMinDistance = 999.0;
	double locFinalDeltaPhi = 999.0, locFinalDeltaZ = 999.0;
	for(size_t loc_i = 0; loc_i < locTrackWireBasedVector.size(); ++loc_i)
	{
		shared_ptr<DBCALShowerMatchParams> locShowerMatchParams;
		double locInputStartTime = locTrackWireBasedVector[loc_i]->t0();
		const vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackWireBasedVector[loc_i]->extrapolations.at(SYS_BCAL);
		if(!locParticleID->Distance_ToTrack(extrapolations, locBCALShower, locInputStartTime, locShowerMatchParams))
			continue;

		double locRSq = locBCALShower->x*locBCALShower->x + locBCALShower->y*locBCALShower->y;
		double locDeltaPhi = locShowerMatchParams->dDeltaPhiToShower;
		double locDeltaZ = locShowerMatchParams->dDeltaZToShower;
		double locDistance = sqrt(locDeltaZ*locDeltaZ + locDeltaPhi*locDeltaPhi*locRSq);
		if(locDistance >= locMinDistance)
			continue;

		locMinDistance = locDistance;
		locFinalDeltaPhi = locDeltaPhi;
		locFinalDeltaZ = locDeltaZ;
	}
	locDetectorMatches->Set_DistanceToNearestTrack(locBCALShower, locFinalDeltaPhi, locFinalDeltaZ);
}

void DDetectorMatches_factory_WireBased::MatchToTrack(const DParticleID* locParticleID, const DFCALShower* locFCALShower, const vector<const DTrackWireBased*>& locTrackWireBasedVector, DDetectorMatches* locDetectorMatches) const
{
	double locMinDistance = 999.0;
	for(size_t loc_i = 0; loc_i < locTrackWireBasedVector.size(); ++loc_i)
	{
		shared_ptr<DFCALShowerMatchParams> locShowerMatchParams;
		double locInputStartTime = locTrackWireBasedVector[loc_i]->t0();
		const vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackWireBasedVector[loc_i]->extrapolations.at(SYS_FCAL);
		if(!locParticleID->Distance_ToTrack(extrapolations, locFCALShower, locInputStartTime, locShowerMatchParams))
			continue;
		if(locShowerMatchParams->dDOCAToShower < locMinDistance)
			locMinDistance = locShowerMatchParams->dDOCAToShower;
	}
	locDetectorMatches->Set_DistanceToNearestTrack(locFCALShower, locMinDistance);
}

void 
DDetectorMatches_factory_WireBased::MatchToFCAL(const DParticleID* locParticleID,
						const DTrackWireBased *locTrackWireBased,
						vector<const DFCALHit *>&locSingleHits,
						DDetectorMatches* locDetectorMatches) const {
  vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackWireBased->extrapolations.at(SYS_FCAL);
  if (extrapolations.size()==0) return;

  for (unsigned int i=0;i<locSingleHits.size();i++){
    double locDOCA=0.,locHitTime;
    if (locParticleID->Distance_ToTrack(locTrackWireBased->t0(),
					extrapolations[0],locSingleHits[i],
					locDOCA,locHitTime)){
      shared_ptr<DFCALSingleHitMatchParams> locMatchParams=std::make_shared<DFCALSingleHitMatchParams>();
      
      locMatchParams->dEHit=locSingleHits[i]->E;
      locMatchParams->dTHit=locHitTime;
      locMatchParams->dFlightTime = extrapolations[0].t;
      locMatchParams->dFlightTimeVariance = 0.; // Fill this in!
      locMatchParams->dPathLength = extrapolations[0].s;
      locMatchParams->dDOCAToHit = locDOCA;
      
      locDetectorMatches->Add_Match(locTrackWireBased,locMatchParams);
    }
  }
}
