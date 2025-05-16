// $Id$
//
//		File: DDetectorMatches_factory.cc
// Created: Tue Aug	9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DDetectorMatches_factory.h"

//------------------
// Init
//------------------
void DDetectorMatches_factory::Init()
{
  ENABLE_FCAL_SINGLE_HITS = false;
  GetApplication()->SetDefaultParameter("PID:ENABLE_FCAL_SINGLE_HITS",ENABLE_FCAL_SINGLE_HITS);

  ENABLE_ECAL_SINGLE_HITS = false;
  GetApplication()->SetDefaultParameter("PID:ENABLE_ECAL_SINGLE_HITS",ENABLE_ECAL_SINGLE_HITS);
}

//------------------
// BeginRun
//------------------
void DDetectorMatches_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	//LEAVE THIS EMPTY!!! OR ELSE WON'T BE INITIALIZED PROPERLY WHEN "COMBO" FACTORY CALLS Create_DDetectorMatches ON REST DATA!!
}

//------------------
// Process
//------------------
void DDetectorMatches_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DTrackTimeBased*> locTrackTimeBasedVector;
	event->Get(locTrackTimeBasedVector);

	DDetectorMatches* locDetectorMatches = Create_DDetectorMatches(event, locTrackTimeBasedVector);
	Insert(locDetectorMatches);
}

DDetectorMatches* DDetectorMatches_factory::Create_DDetectorMatches(const std::shared_ptr<const JEvent>& event, vector<const DTrackTimeBased*>& locTrackTimeBasedVector)
{
	const DParticleID* locParticleID = NULL;
	event->GetSingle(locParticleID);

	vector<const DSCHit*> locSCHits;
	event->Get(locSCHits);

	vector<const DTOFPoint*> locTOFPoints;
	event->Get(locTOFPoints);

	vector<const DFCALShower*> locFCALShowers;
	event->Get(locFCALShowers);

	vector<const DECALShower*> locECALShowers;
	event->Get(locECALShowers);

	vector<const DBCALShower*> locBCALShowers;
	event->Get(locBCALShowers);

	vector<const DDIRCPmtHit*> locDIRCHits;
	event->Get(locDIRCHits);

	// cheat and get truth info of track at bar
	vector<const DDIRCTruthBarHit*> locDIRCBarHits;
	event->Get(locDIRCBarHits);

	vector<const DCTOFPoint*> locCTOFPoints;
	event->Get(locCTOFPoints);

	vector<const DFMWPCCluster *> locFMWPCClusters;
	event->Get(locFMWPCClusters);

	vector<const DTRDSegment *> locTRDSegments;
	event->Get(locTRDSegments);

	DDetectorMatches* locDetectorMatches = new DDetectorMatches();

	//Match tracks to showers/hits
	for(size_t loc_i = 0; loc_i < locTrackTimeBasedVector.size(); ++loc_i)
	{
		MatchToBCAL(locParticleID, locTrackTimeBasedVector[loc_i], locBCALShowers, locDetectorMatches);
		MatchToTOF(locParticleID, locTrackTimeBasedVector[loc_i], locTOFPoints, locDetectorMatches);
		MatchToFCAL(locParticleID, locTrackTimeBasedVector[loc_i], locFCALShowers, locDetectorMatches);
		MatchToSC(locParticleID, locTrackTimeBasedVector[loc_i], locSCHits, locDetectorMatches);
		MatchToDIRC(locParticleID, locTrackTimeBasedVector[loc_i], locDIRCHits, locDetectorMatches, locDIRCBarHits);
		if (locTrackTimeBasedVector[loc_i]->PID()<10){ // GEANT ids; ignore proton=14 and kaons=11+12
		  MatchToCTOF(locParticleID, locTrackTimeBasedVector[loc_i], locCTOFPoints, locDetectorMatches);
		  MatchToFMWPC(locTrackTimeBasedVector[loc_i], locFMWPCClusters, locDetectorMatches);
		}
		MatchToTRD(locParticleID, locTrackTimeBasedVector[loc_i], locTRDSegments, locDetectorMatches);
		MatchToECAL(locParticleID, locTrackTimeBasedVector[loc_i], locECALShowers, locDetectorMatches);
	}

	//Find nearest tracks to showers
	for(size_t loc_i = 0; loc_i < locBCALShowers.size(); ++loc_i)
		MatchToTrack(locParticleID, locBCALShowers[loc_i], locTrackTimeBasedVector, locDetectorMatches);
	for(size_t loc_i = 0; loc_i < locFCALShowers.size(); ++loc_i)
		MatchToTrack(locParticleID, locFCALShowers[loc_i], locTrackTimeBasedVector, locDetectorMatches);
	for(size_t loc_i = 0; loc_i < locECALShowers.size(); ++loc_i)
		MatchToTrack(locParticleID, locECALShowers[loc_i], locTrackTimeBasedVector, locDetectorMatches);

	// Try to find matches between tracks and single hits in FCAL
	if (ENABLE_FCAL_SINGLE_HITS){
	  vector<const DFCALHit*> locFCALHits;
	  event->Get(locFCALHits);
	  if (locFCALHits.size()>0){
	    vector<const DFCALHit*>locSingleHits;
	    locParticleID->GetSingleFCALHits(locFCALShowers,locFCALHits,
					     locSingleHits);
	    for (size_t loc_j=0;loc_j<locTrackTimeBasedVector.size();loc_j++){
	      MatchToFCAL(locParticleID,locTrackTimeBasedVector[loc_j],
			  locSingleHits,locDetectorMatches);
	    }
	  }
	}

	// Try to find matches between tracks and single hits in ECAL
	if (ENABLE_ECAL_SINGLE_HITS){
	  vector<const DECALHit*> locECALHits;
	  event->Get(locECALHits);
	  if (locECALHits.size()>0){
	    vector<const DECALHit*>locSingleHits;
	    locParticleID->GetSingleECALHits(locECALShowers,locECALHits,
					     locSingleHits);
	    for (size_t loc_j=0;loc_j<locTrackTimeBasedVector.size();loc_j++){
	      MatchToECAL(locParticleID,locTrackTimeBasedVector[loc_j],
			  locSingleHits,locDetectorMatches);
	    }
	  }
	}

	//Set flight-time/p correlations
	for(size_t loc_i = 0; loc_i < locTrackTimeBasedVector.size(); ++loc_i)
	{
		double locFlightTimePCorrelation = locParticleID->Calc_BCALFlightTimePCorrelation(locTrackTimeBasedVector[loc_i], locDetectorMatches);
		if(isfinite(locFlightTimePCorrelation))
			locDetectorMatches->Set_FlightTimePCorrelation(locTrackTimeBasedVector[loc_i], SYS_BCAL, locFlightTimePCorrelation);

		locFlightTimePCorrelation = locParticleID->Calc_TOFFlightTimePCorrelation(locTrackTimeBasedVector[loc_i], locDetectorMatches);
		if(isfinite(locFlightTimePCorrelation))
			locDetectorMatches->Set_FlightTimePCorrelation(locTrackTimeBasedVector[loc_i], SYS_TOF, locFlightTimePCorrelation);

		locFlightTimePCorrelation = locParticleID->Calc_FCALFlightTimePCorrelation(locTrackTimeBasedVector[loc_i], locDetectorMatches);
		if(isfinite(locFlightTimePCorrelation))
			locDetectorMatches->Set_FlightTimePCorrelation(locTrackTimeBasedVector[loc_i], SYS_FCAL, locFlightTimePCorrelation);

		locFlightTimePCorrelation = locParticleID->Calc_SCFlightTimePCorrelation(locTrackTimeBasedVector[loc_i], locDetectorMatches);
		if(isfinite(locFlightTimePCorrelation))
			locDetectorMatches->Set_FlightTimePCorrelation(locTrackTimeBasedVector[loc_i], SYS_START, locFlightTimePCorrelation);
	}

	return locDetectorMatches;
}

void DDetectorMatches_factory::MatchToBCAL(const DParticleID* locParticleID, const DTrackTimeBased* locTrackTimeBased, const vector<const DBCALShower*>& locBCALShowers, DDetectorMatches* locDetectorMatches) const
{
  vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackTimeBased->extrapolations.at(SYS_BCAL);
	if (extrapolations.size()==0) return;

	double locInputStartTime = locTrackTimeBased->t0();
	for(size_t loc_i = 0; loc_i < locBCALShowers.size(); ++loc_i)
	{
	  shared_ptr<DBCALShowerMatchParams> locShowerMatchParams;
	  if(locParticleID->Cut_MatchDistance(extrapolations, locBCALShowers[loc_i], locInputStartTime, locShowerMatchParams))
	    locDetectorMatches->Add_Match(locTrackTimeBased, locBCALShowers[loc_i], locShowerMatchParams);
	}
}

void DDetectorMatches_factory::MatchToFMWPC(const DTrackTimeBased* locTrackTimeBased, const vector<const DFMWPCCluster*>& locFMWPCClusters, DDetectorMatches* locDetectorMatches) const{
  auto fmwpc_projections=locTrackTimeBased->extrapolations.at(SYS_FMWPC);
  if (fmwpc_projections.size()==0) return;
  
  // Loop over projections filling in arrays of matching info
  vector<int>locLayers;
  vector<int>locNhits;
  vector<int>locDists;
  vector<int>locClosestWires;
  const double FMWPC_WIRE_SPACING=1.016; //cm
  bool got_match=false;
  for( int layer=1; layer<=(int)fmwpc_projections.size(); layer++){
    auto proj = fmwpc_projections[layer-1];
    // x and y projections from track
    double xpos=proj.position.x();
    double ypos=proj.position.y();
 
    // Loop over DFMWPCClusters and find closest match to this projection
    int min_dist = 1000000;
    int wire_trk_proj=0;
    const DFMWPCCluster* closest_fmwpc_cluster= nullptr;
    for(auto fmwpccluster : locFMWPCClusters){
      if( fmwpccluster->layer != layer ) continue;
      
      // Convert into local coordinates so we can work with wire numbers
      double s=fmwpccluster->orientation==DGeometry::kFMWPC_WIRE_ORIENTATION_VERTICAL ? xpos+fmwpccluster->xoffset : ypos+fmwpccluster->yoffset;
      wire_trk_proj = round(71.5 + s/FMWPC_WIRE_SPACING) + 1; // 1-144
      
      // If the projection is outside of the wire range then bail now
      if( (wire_trk_proj<1) || (wire_trk_proj>144) ) continue; 

      int dist=1000000;
      if( wire_trk_proj >= fmwpccluster->first_wire ){
	dist = wire_trk_proj - fmwpccluster->last_wire; // distance beyond last wire (will be negative if inside cluster)
	if( dist < 0 ) dist = 0; // force dist to 0 if inside cluster
      }else{
	dist = fmwpccluster->first_wire - wire_trk_proj; // distance before first wire
      }
      
      if( dist < min_dist ){
	min_dist = dist;
	closest_fmwpc_cluster = fmwpccluster;
      }
    }
  
    // If a DFMWPCCluster was found, add the match info to the track
    if( closest_fmwpc_cluster ){
      int closest_wire=wire_trk_proj;
      if (wire_trk_proj < closest_fmwpc_cluster->first_wire ) {
	closest_wire = closest_fmwpc_cluster->first_wire;
      }
      else if (wire_trk_proj > closest_fmwpc_cluster->last_wire){
	closest_wire = closest_fmwpc_cluster->last_wire;
      }
      locLayers.push_back(layer);
      locNhits.push_back(closest_fmwpc_cluster->Nhits);
      locDists.push_back(min_dist);
      locClosestWires.push_back(closest_wire);

      got_match=true;
    }
  }
  if (got_match){
    shared_ptr<DFMWPCMatchParams>locFMWPCMatchParams=std::make_shared<DFMWPCMatchParams>();
    locFMWPCMatchParams->dLayers=locLayers;
    locFMWPCMatchParams->dNhits=locNhits;
    locFMWPCMatchParams->dDists=locDists;
    locFMWPCMatchParams->dClosestWires=locClosestWires;
    locDetectorMatches->Add_Match(locTrackTimeBased,locFMWPCMatchParams);
  }
}

void DDetectorMatches_factory::MatchToCTOF(const DParticleID* locParticleID, const DTrackTimeBased* locTrackTimeBased, const vector<const DCTOFPoint*>& locCTOFPoints, DDetectorMatches* locDetectorMatches) const
{
  vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_CTOF);
  if (extrapolations.size()==0) return;
  
  double locInputStartTime = locTrackTimeBased->t0();
  for(size_t loc_i = 0; loc_i < locCTOFPoints.size(); ++loc_i)
    {
      shared_ptr<DCTOFHitMatchParams> locCTOFHitMatchParams;
      if(locParticleID->Cut_MatchDistance(extrapolations, locCTOFPoints[loc_i], locInputStartTime, locCTOFHitMatchParams))
	locDetectorMatches->Add_Match(locTrackTimeBased, locCTOFPoints[loc_i], locCTOFHitMatchParams);
    }
}

void DDetectorMatches_factory::MatchToTOF(const DParticleID* locParticleID, const DTrackTimeBased* locTrackTimeBased, const vector<const DTOFPoint*>& locTOFPoints, DDetectorMatches* locDetectorMatches) const
{
  vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_TOF);
	if (extrapolations.size()==0) return;

	double locInputStartTime = locTrackTimeBased->t0();
	for(size_t loc_i = 0; loc_i < locTOFPoints.size(); ++loc_i)
	{
	  shared_ptr<DTOFHitMatchParams> locTOFHitMatchParams;
	  if(locParticleID->Cut_MatchDistance(extrapolations, locTOFPoints[loc_i], locInputStartTime, locTOFHitMatchParams))
	    locDetectorMatches->Add_Match(locTrackTimeBased, locTOFPoints[loc_i], locTOFHitMatchParams);
	}
}

void DDetectorMatches_factory::MatchToFCAL(const DParticleID* locParticleID, const DTrackTimeBased* locTrackTimeBased, const vector<const DFCALShower*>& locFCALShowers, DDetectorMatches* locDetectorMatches) const
{
  vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_FCAL);
	if (extrapolations.size()==0) return;

	double locInputStartTime = locTrackTimeBased->t0();
	for(size_t loc_i = 0; loc_i < locFCALShowers.size(); ++loc_i)
	{
	  shared_ptr<DFCALShowerMatchParams>locShowerMatchParams;
	  if(locParticleID->Cut_MatchDistance(extrapolations, locFCALShowers[loc_i], locInputStartTime, locShowerMatchParams))
	    locDetectorMatches->Add_Match(locTrackTimeBased, locFCALShowers[loc_i], locShowerMatchParams);
	}
}

void DDetectorMatches_factory::MatchToECAL(const DParticleID* locParticleID, const DTrackTimeBased* locTrackTimeBased, const vector<const DECALShower*>& locECALShowers, DDetectorMatches* locDetectorMatches) const
{
  vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_ECAL);
  if (extrapolations.size()==0) return;

  double locInputStartTime = locTrackTimeBased->t0();
  for(size_t loc_i = 0; loc_i < locECALShowers.size(); ++loc_i)
    {
      shared_ptr<DECALShowerMatchParams>locShowerMatchParams;
      if(locParticleID->Cut_MatchDistance(extrapolations, locECALShowers[loc_i], locInputStartTime, locShowerMatchParams))
	    locDetectorMatches->Add_Match(locTrackTimeBased, locECALShowers[loc_i], locShowerMatchParams);
	}
}

void DDetectorMatches_factory::MatchToSC(const DParticleID* locParticleID, const DTrackTimeBased* locTrackTimeBased, const vector<const DSCHit*>& locSCHits, DDetectorMatches* locDetectorMatches) const
{
  vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_START);
	if (extrapolations.size()==0) return;

	double locInputStartTime = locTrackTimeBased->t0();
	for(size_t loc_i = 0; loc_i < locSCHits.size(); ++loc_i)
	{
	    shared_ptr<DSCHitMatchParams>locSCHitMatchParams;
	    if(locParticleID->Cut_MatchDistance(extrapolations, locSCHits[loc_i], locInputStartTime, locSCHitMatchParams, true))
	      locDetectorMatches->Add_Match(locTrackTimeBased, locSCHits[loc_i], locSCHitMatchParams);
	}
}

void DDetectorMatches_factory::MatchToDIRC(const DParticleID* locParticleID, const DTrackTimeBased* locTrackTimeBased, const vector<const DDIRCPmtHit*>& locDIRCHits, DDetectorMatches* locDetectorMatches, const vector<const DDIRCTruthBarHit*>& locDIRCBarHits) const
{
	vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_DIRC);
	if(extrapolations.size()==0) return;

	double locInputStartTime = locTrackTimeBased->time();

	// objects to hold DIRC match parameters and links between tracks and DIRC hits
	shared_ptr<DDIRCMatchParams> locDIRCMatchParams;
	map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> > locDIRCTrackMatchParams;
	locDetectorMatches->Get_DIRCTrackMatchParamsMap(locDIRCTrackMatchParams);

	// run DIRC LUT algorithm and add detector match
	if(locParticleID->Cut_MatchDIRC(extrapolations, locDIRCHits, locInputStartTime, locTrackTimeBased->PID(), locDIRCMatchParams, locDIRCBarHits, locDIRCTrackMatchParams))
		locDetectorMatches->Add_Match(locTrackTimeBased, locDIRCMatchParams);
}

void DDetectorMatches_factory::MatchToTRD(const DParticleID* locParticleID, const DTrackTimeBased* locTrackTimeBased, const vector<const DTRDSegment*>& locTRDSegments, DDetectorMatches* locDetectorMatches) const
{
  vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_TRD);
  if (extrapolations.size()==0) return;

  for(size_t loc_i = 0; loc_i < locTRDSegments.size(); ++loc_i){
    shared_ptr<DTRDMatchParams> locTRDMatchParams;
    if(locParticleID->Cut_MatchDistance(extrapolations, locTRDSegments[loc_i], locTRDMatchParams))
      locDetectorMatches->Add_Match(locTrackTimeBased, locTRDMatchParams);
  }
}

void DDetectorMatches_factory::MatchToTrack(const DParticleID* locParticleID, const DBCALShower* locBCALShower, const vector<const DTrackTimeBased*>& locTrackTimeBasedVector, DDetectorMatches* locDetectorMatches) const
{
	double locMinDistance = 999.0;
	double locFinalDeltaPhi = 999.0, locFinalDeltaZ = 999.0;
	for(size_t loc_i = 0; loc_i < locTrackTimeBasedVector.size(); ++loc_i)
	{
		shared_ptr<DBCALShowerMatchParams> locShowerMatchParams;
		double locInputStartTime = locTrackTimeBasedVector[loc_i]->t0();
		
		map<DetectorSystem_t,vector<DTrackFitter::Extrapolation_t> >extrapolations=locTrackTimeBasedVector[loc_i]->extrapolations;
		if (extrapolations.size()==0) continue;

		if(!locParticleID->Distance_ToTrack(extrapolations.at(SYS_BCAL), locBCALShower, locInputStartTime, locShowerMatchParams))
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

void DDetectorMatches_factory::MatchToTrack(const DParticleID* locParticleID, const DFCALShower* locFCALShower, const vector<const DTrackTimeBased*>& locTrackTimeBasedVector, DDetectorMatches* locDetectorMatches) const
{
	double locMinDistance = 999.0;
	for(size_t loc_i = 0; loc_i < locTrackTimeBasedVector.size(); ++loc_i)
	{
	  map<DetectorSystem_t,vector<DTrackFitter::Extrapolation_t> >extrapolations=locTrackTimeBasedVector[loc_i]->extrapolations;
	  if (extrapolations.size()==0) return;

	  shared_ptr<DFCALShowerMatchParams> locShowerMatchParams;
	  double locInputStartTime = locTrackTimeBasedVector[loc_i]->t0();
	  if(!locParticleID->Distance_ToTrack(extrapolations.at(SYS_FCAL), locFCALShower, locInputStartTime, locShowerMatchParams))
	    continue;
	  if(locShowerMatchParams->dDOCAToShower < locMinDistance)
			locMinDistance = locShowerMatchParams->dDOCAToShower;
	}
	locDetectorMatches->Set_DistanceToNearestTrack(locFCALShower, locMinDistance);
}

void DDetectorMatches_factory::MatchToTrack(const DParticleID* locParticleID, const DECALShower* locECALShower, const vector<const DTrackTimeBased*>& locTrackTimeBasedVector, DDetectorMatches* locDetectorMatches) const
{
  double locMinDistance = 999.0;
  for(size_t loc_i = 0; loc_i < locTrackTimeBasedVector.size(); ++loc_i)
    {
      map<DetectorSystem_t,vector<DTrackFitter::Extrapolation_t> >extrapolations=locTrackTimeBasedVector[loc_i]->extrapolations;
      if (extrapolations.size()==0) return;
      
      shared_ptr<DECALShowerMatchParams> locShowerMatchParams;
      double locInputStartTime = locTrackTimeBasedVector[loc_i]->t0();
      if(!locParticleID->Distance_ToTrack(extrapolations.at(SYS_ECAL), locECALShower, locInputStartTime, locShowerMatchParams))
	continue;
      if(locShowerMatchParams->dDOCAToShower < locMinDistance)
	locMinDistance = locShowerMatchParams->dDOCAToShower;
    }
  locDetectorMatches->Set_DistanceToNearestTrack(locECALShower, locMinDistance);
}

// Try to find matches between a track and a single hit in FCAL
void 
DDetectorMatches_factory::MatchToFCAL(const DParticleID* locParticleID,
				      const DTrackTimeBased *locTrackTimeBased,
				      vector<const DFCALHit *>&locSingleHits,
				      DDetectorMatches* locDetectorMatches) const {
  vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_FCAL);
  if (extrapolations.size()==0) return;

  for (unsigned int i=0;i<locSingleHits.size();i++){
    double locDOCA=0.,locHitTime;
    if (locParticleID->Distance_ToTrack(locTrackTimeBased->t0(),
					extrapolations[0],locSingleHits[i],
					locDOCA,locHitTime)){
      shared_ptr<DFCALSingleHitMatchParams> locMatchParams=std::make_shared<DFCALSingleHitMatchParams>();
      
      locMatchParams->dEHit=locSingleHits[i]->E;
      locMatchParams->dTHit=locHitTime;
      locMatchParams->dFlightTime = extrapolations[0].t;
      locMatchParams->dFlightTimeVariance = 0.; // Fill this in!
      locMatchParams->dPathLength = extrapolations[0].s;
      locMatchParams->dDOCAToHit = locDOCA;
      
      locDetectorMatches->Add_Match(locTrackTimeBased,locMatchParams);
    }
  }
}

// Try to find matches between a track and a single hit in ECAL
void DDetectorMatches_factory::MatchToECAL(const DParticleID* locParticleID,
					   const DTrackTimeBased *locTrackTimeBased,
					   vector<const DECALHit *>&locSingleHits,
					   DDetectorMatches* locDetectorMatches) const {
  vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_ECAL);
  if (extrapolations.size()==0) return;

  for (unsigned int i=0;i<locSingleHits.size();i++){
    double locDOCA=0.,locHitTime;
    if (locParticleID->Distance_ToTrack(locTrackTimeBased->t0(),
					extrapolations[0],locSingleHits[i],
					locDOCA,locHitTime)){
      shared_ptr<DECALSingleHitMatchParams> locMatchParams=std::make_shared<DECALSingleHitMatchParams>();
      
      locMatchParams->dEHit=locSingleHits[i]->E;
      locMatchParams->dTHit=locHitTime;
      locMatchParams->dFlightTime = extrapolations[0].t;
      locMatchParams->dFlightTimeVariance = 0.; // Fill this in!
      locMatchParams->dPathLength = extrapolations[0].s;
      locMatchParams->dDOCAToHit = locDOCA;
      
      locDetectorMatches->Add_Match(locTrackTimeBased,locMatchParams);
    }
  }
}

