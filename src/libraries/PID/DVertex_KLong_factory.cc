// $Id$
//
//    File: DVertex_KLong_factory.cc
//

#include "DVertex_KLong_factory.h"
#include "PID/DChargedTrack.h"
#include "PID/DChargedTrackHypothesis.h"

#include "DANA/DEvent.h"

//------------------
// Init
//------------------
void DVertex_KLong_factory::Init(void)
{
	dKinFitDebugLevel = 0;
	dMinTrackingFOM = 5.73303E-7;
	dNoKinematicFitFlag = false;
	dForceTargetCenter = false;
	dUseWeightedAverage = false;
	dUseStartCounterTimesOnly = false;
	dMinTrackingNDF = 7;

	auto app = GetApplication();
	app->SetDefaultParameter("VERTEX:USE_TARGET_CENTER", dForceTargetCenter);
	app->SetDefaultParameter("VERTEX:NO_KINFIT_FLAG", dNoKinematicFitFlag);
	app->SetDefaultParameter("VERTEX:DEBUGLEVEL", dKinFitDebugLevel);
	app->SetDefaultParameter("VERTEX:MINTRACKINGFOM", dMinTrackingFOM);
	app->SetDefaultParameter("VERTEX:MINTRACKNDF", dMinTrackingNDF);
	app->SetDefaultParameter("VERTEX:USEWEIGHTEDAVERAGE", dUseWeightedAverage);
	app->SetDefaultParameter("VERTEX:USESCTIMESONLY", dUseStartCounterTimesOnly);

}

//------------------
// BeginRun
//------------------
void DVertex_KLong_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	dKinFitUtils = new DKinFitUtils_GlueX(event);
	dKinFitter = new DKinFitter(dKinFitUtils);
	dKinFitUtils->Set_UpdateCovarianceMatricesFlag(false);

	// Get Target parameters from XML
	dTargetZCenter = 65.0;
	dTargetLength = 35.0;
	dTargetRadius = 3.0; //FIX: grab from database!!!
	m_beamSpotX = 0;
	m_beamSpotY = 0;
	
	DGeometry *locGeometry = DEvent::GetDGeometry(event);
	locGeometry->GetTargetZ(dTargetZCenter);
	locGeometry->GetTargetLength(dTargetLength);
	
	std::map<string, float> beam_spot;
	DEvent::GetCalib(event, "PHOTON_BEAM/beam_spot", beam_spot);
	m_beamSpotX = beam_spot.at("x");
	m_beamSpotY = beam_spot.at("y");

	dKinFitter->Set_DebugLevel(dKinFitDebugLevel);

	event->GetSingle(dAnalysisUtilities);

}

//------------------
// Process
//------------------
void DVertex_KLong_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	// For KLong reconstruction, the beam is massive, so we figure out the vertex time based on
	// the reconstructed tracks, and figure out the KLong momentum from this

	//preferentially (kinematic fit):
		//use tracks with a matched hit & good tracking FOM
		//if no good tracks (or none with matched hits), use all tracks
		//if no tracks, use target center

 	vector<const DTrackTimeBased*> locTrackTimeBasedVector;
 	event->Get(locTrackTimeBasedVector);

// 	vector<const DChargedTrack*> locChargedTrackVector;
// 	locEventLoop->Get(locChargedTrackVector, "KLVertex");
	//locEventLoop->Get(locChargedTrackVector);

	const DDetectorMatches* locDetectorMatches = NULL;
	event->GetSingle(locDetectorMatches);

	const DParticleID* locParticleID = NULL;
	event->GetSingle(locParticleID);

	// give option for just using the target center, e.g. if the magnetic
	// field is off and/or tracking is otherwise not working well
	if(dForceTargetCenter) {
		Create_Vertex_NoTracks();
		return;
	}

// 	select the best DTrackTimeBased for each track: use best tracking FOM
// 	map<oid_t, const DTrackTimeBased*> locBestTrackTimeBasedMap; //lowest tracking chisq/ndf for each candidate id
// 	for(size_t loc_i = 0; loc_i < locTrackTimeBasedVector.size(); ++loc_i)
// 	{
// 		oid_t locCandidateID = locTrackTimeBasedVector[loc_i]->candidateid;
// 		if(locBestTrackTimeBasedMap.find(locCandidateID) == locBestTrackTimeBasedMap.end())
// 			locBestTrackTimeBasedMap[locCandidateID] = locTrackTimeBasedVector[loc_i];
// 		else if(locTrackTimeBasedVector[loc_i]->FOM > locBestTrackTimeBasedMap[locCandidateID]->FOM)
// 			locBestTrackTimeBasedMap[locCandidateID] = locTrackTimeBasedVector[loc_i];
// 	}
/**
	//separate the tracks based on high/low tracking FOM & has hit-match
	//map<oid_t, const DTrackTimeBased*>::iterator locIterator;
	vector<const DTrackTimeBased*> locTrackTimeBasedVector_OnePerTrack, locTrackTimeBasedVector_OnePerTrack_Good;
	vector<const DKinematicData*> locKinematicDataVector_OnePerTrack, locKinematicDataVector_OnePerTrack_Good;
	//for(locIterator = locBestTrackTimeBasedMap.begin(); locIterator != locBestTrackTimeBasedMap.end(); ++locIterator)
	for(auto locTrack : locChargedTrackVector)
	{
		const DChargedTrackHypothesis* locChargedTrackHypothesis = locTrack->Get_BestTrackingFOM();
		const DTrackTimeBased* locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();
		
		if(locTrackTimeBased->Ndof < dMinTrackingNDF) continue;
		
		locTrackTimeBasedVector_OnePerTrack.push_back(locTrackTimeBased);
		locKinematicDataVector_OnePerTrack.push_back(locChargedTrackHypothesis);
		if((locTrackTimeBased->FOM >= dMinTrackingFOM) && locDetectorMatches->Get_IsMatchedToHit(locTrackTimeBased)) {
			locTrackTimeBasedVector_OnePerTrack_Good.push_back(locTrackTimeBased);
			locKinematicDataVector_OnePerTrack_Good.push_back(locChargedTrackHypothesis);
		}
	}
**/

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

	//separate the tracks based on high/low tracking FOM & has hit-match
	map<oid_t, const DTrackTimeBased*>::iterator locIterator;
	vector<const DTrackTimeBased*> locTrackTimeBasedVector_OnePerTrack, locTrackTimeBasedVector_OnePerTrack_Good;
	vector<const DKinematicData*> locKinematicDataVector_OnePerTrack, locKinematicDataVector_OnePerTrack_Good;
 	vector< shared_ptr<DTrackTimeBased> > locTrackTimeBasedVector_Saved;    // memory management
	for(locIterator = locBestTrackTimeBasedMap.begin(); locIterator != locBestTrackTimeBasedMap.end(); ++locIterator)
	{
		//DTrackTimeBased* locTrackTimeBased = new DTrackTimeBased(*locIterator->second);
		shared_ptr<DTrackTimeBased> locTrackTimeBased(new DTrackTimeBased(*locIterator->second));
		locTrackTimeBased->setErrorMatrix(locIterator->second->errorMatrix());
		locTrackTimeBasedVector_Saved.push_back(locTrackTimeBased);
		
		// dislike the use of locIterator->second
		Set_TrackTime(event, locTrackTimeBased.get(), locIterator->second, locDetectorMatches, locParticleID);
		
		locTrackTimeBasedVector_OnePerTrack.push_back(const_cast<DTrackTimeBased*>(locTrackTimeBased.get()));
		locKinematicDataVector_OnePerTrack.push_back(const_cast<DKinematicData*>(static_cast<DKinematicData*>(locTrackTimeBased.get())));
		if((locTrackTimeBased->FOM >= dMinTrackingFOM) && locDetectorMatches->Get_IsMatchedToHit(locTrackTimeBased.get())) {
			locTrackTimeBasedVector_OnePerTrack_Good.push_back(locTrackTimeBased.get());
			locKinematicDataVector_OnePerTrack_Good.push_back(locTrackTimeBased.get());
		}
	}


	vector<const DTrackTimeBased*> locTrackTimeBasedVectorToUse = (locTrackTimeBasedVector_OnePerTrack_Good.size() >= 2) ? locTrackTimeBasedVector_OnePerTrack_Good : locTrackTimeBasedVector_OnePerTrack;
	vector<const DKinematicData*> locKinematicDataVectorToUse = (locKinematicDataVector_OnePerTrack_Good.size() >= 2) ? locKinematicDataVector_OnePerTrack : locKinematicDataVector_OnePerTrack;

	//handle cases of no/one track
	if(locTrackTimeBasedVectorToUse.empty()) {
		Create_Vertex_NoTracks();
		return;
	}
	if(locTrackTimeBasedVectorToUse.size() == 1) {
		Create_Vertex_OneTrack(locTrackTimeBasedVectorToUse[0]);
		return;
	}
	
	// first calculate a rough vertex
	DVector3 locRoughPosition = dAnalysisUtilities->Calc_CrudeVertex(locTrackTimeBasedVectorToUse);
	
// 	cout << " rough position = " << locRoughPosition.x() << "  " << locRoughPosition.y() << "  " << locRoughPosition.z() << endl;
// 	cout << " crude time = " << dAnalysisUtilities->Calc_CrudeTime(locKinematicDataVectorToUse, const_cast<DVector3&>(locRoughPosition)) << endl;
// 	cout << " crude weighted time = " << dAnalysisUtilities->Calc_CrudeWeightedTime(locKinematicDataVectorToUse, const_cast<DVector3&>(locRoughPosition)) << endl;
	
	double locRoughTime = 0.;
	// can't use weighted average until we propagate the variances...
	if(dUseWeightedAverage)
		locRoughTime = dAnalysisUtilities->Calc_CrudeWeightedTime(locKinematicDataVectorToUse, const_cast<DVector3&>(locRoughPosition));
	else
		locRoughTime = dAnalysisUtilities->Calc_CrudeTime(locKinematicDataVectorToUse, const_cast<DVector3&>(locRoughPosition));
//	double locRoughTime = dAnalysisUtilities->Calc_CrudeTime(locKinematicDataVectorToUse, const_cast<DVector3&>(locRoughPosition));

	// if only want rough guess, save it and exit
	if(dNoKinematicFitFlag || (locTrackTimeBasedVectorToUse[0]->errorMatrix() == nullptr)) {
		Create_Vertex_Rough(locRoughPosition, locRoughTime);
		return;
	}

	//prepare for kinematic fit
	dKinFitUtils->Reset_NewEvent();
	dKinFitter->Reset_NewEvent();
	TVector3 locTRoughPosition(locRoughPosition.X(), locRoughPosition.Y(), locRoughPosition.Z());

	// create particles for kinematic fit
	set<shared_ptr<DKinFitParticle>> locKinFitParticles;
	for(size_t loc_i = 0; loc_i < locTrackTimeBasedVectorToUse.size(); ++loc_i)
		locKinFitParticles.insert(dKinFitUtils->Make_DetectedParticle(locTrackTimeBasedVectorToUse[loc_i]));

	// create vertex constraint
	auto locVertexConstraint = dKinFitUtils->Make_VertexConstraint(locKinFitParticles, {}, locTRoughPosition);
	dKinFitter->Add_Constraint(locVertexConstraint);

	// fit, save, and return
	if(!dKinFitter->Fit_Reaction()) { //if fit fails to converge: use rough results
// 		cout << "Using rough vertex" << endl;
		Create_Vertex_Rough(locRoughPosition, locRoughTime);
		return;
	}

// 	cout << "Using good vertex" << endl;
	//save kinfit results
	Create_Vertex_KinFit(locKinematicDataVectorToUse);
}


// double DVertex_KLong_factory::Calc_CrudeVertexTime(const vector<DKinFitParticle*>& locParticles, const DVector3& locCommonVertex) const
// {
// // 	//first crudely estimate the beam velocity
// //     // distance between KPT and dTargetCenterZ is assumed to be 24 m
// // 	const double c = 29.9792458;
// //     double KL_distance = ( locCommonVertex - dTargetCenterZ ) + ( 24. * 100. );
// // 	double KL_propagation_time = 0.;
// // 	for(size_t loc_i = 0; loc_i < locParticles.size(); ++loc_i)
// // 		KL_propagation_time
// //     double beta = (KL_distance / KL_propagation_time) / c;
// //     
// //     if(beta > 1.)  // don't break special relativity
// //     	beta = 1;
// 
// 	//crudely propagate the track times to the common vertex and return the average track time
// 	DVector3 locPOCA;
// 	DVector3 locDeltaVertex;
// 	double locAverageTime = 0.0;
// 	for(size_t loc_i = 0; loc_i < locParticles.size(); ++loc_i)
// 	{
// 		double locTime = 0.0;
// 		double locE = locParticles[loc_i]->Get_ShowerEnergy();
// 		if((locParticles[loc_i]->Get_Charge() == 0) && (locE > 0.0))
// 		{
// 			double locMass = locParticles[loc_i]->Get_Mass();
// 			double locPMag = sqrt(locE*locE - locMass*locMass);
// 			DVector3 locPosition = locParticles[loc_i]->Get_Position();
// 			DVector3 locDPosition(locPosition.X(), locPosition.Y(), locPosition.Z());
// 			DVector3 locDeltaVertex = locDPosition - locCommonVertex;
// 			locTime = locParticles[loc_i]->Get_Time() + locDeltaVertex.Mag()*locE/(beta*29.9792458*locPMag);
// 		}
// 		else
// 		{
// 			Calc_DOCAToVertex(locParticles[loc_i], locCommonVertex, locPOCA);
// 			locDeltaVertex = locPOCA - DVector3(locParticles[loc_i]->Get_Position().X(),locParticles[loc_i]->Get_Position().Y(),locParticles[loc_i]->Get_Position().Z());
// 			DVector3 locMomentum(locParticles[loc_i]->Get_Momentum().X(),locParticles[loc_i]->Get_Momentum().Y(),locParticles[loc_i]->Get_Momentum().Z());
// 			locTime = locParticles[loc_i]->Get_Time() + locDeltaVertex.Dot(locMomentum)*locParticles[loc_i]->Get_Energy()/(beta*29.9792458*locMomentum.Mag2());
// 		}
// 		locAverageTime += locTime;
// 	}
// 	return locAverageTime/(double(locParticles.size()));
// }

void DVertex_KLong_factory::Set_TrackTime(const std::shared_ptr<const JEvent>& event, DTrackTimeBased* locTrackTimeBased, const DTrackTimeBased* locTrackTimeBased_ToMatch,  const DDetectorMatches* locDetectorMatches, const DParticleID* locPIDAlgorithm)
{
	shared_ptr<const DSCHitMatchParams> locSCHitMatchParams;
	shared_ptr<const DBCALShowerMatchParams> locBCALShowerMatchParams;
	shared_ptr<const DTOFHitMatchParams> locTOFHitMatchParams;
	shared_ptr<const DFCALShowerMatchParams> locFCALShowerMatchParams;
	
	vector<shared_ptr<const DSCHitMatchParams> > locSCHitMatchParamsVec;
	locDetectorMatches->Get_SCMatchParams(locTrackTimeBased_ToMatch, locSCHitMatchParamsVec);

	shared_ptr<TMatrixFSym> locCovarianceMatrix( new TMatrixFSym(*locTrackTimeBased->errorMatrix()) );

	// default
	(*locCovarianceMatrix)(6,6) = 9.0e9;

// cout << "DVertex_KLong_factory::Set_TrackTime()" << endl;
// cout << " starting time = " << locTrackTimeBased->time() << endl;
// cout << " candidate = " << locTrackTimeBased->candidateid << endl;
// cout << " PID = " << locTrackTimeBased->PID() << endl;
// 
// cout << " SC check = " << locPIDAlgorithm->Get_BestSCMatchParams(locTrackTimeBased_ToMatch, locDetectorMatches, locSCHitMatchParams) << endl;
// cout << " num SC matches = " << locSCHitMatchParamsVec.size() << endl;
// //cout << " num SC matches = " << locDetectorMatches->Get_NumTrackSCMatches() << endl;
// 

	if(dUseStartCounterTimesOnly) {
		if(locPIDAlgorithm->Get_BestSCMatchParams(locTrackTimeBased_ToMatch, locDetectorMatches, locSCHitMatchParams))
		{
			double locPropagatedTime = locSCHitMatchParams->dHitTime - locSCHitMatchParams->dFlightTime;
			locTrackTimeBased->setTime(locPropagatedTime);
			(*locCovarianceMatrix)(6,6) = 0.3*0.3+locSCHitMatchParams->dFlightTimeVariance;
		}	
	} else {
		// BCAL
		if(locPIDAlgorithm->Get_BestBCALMatchParams(locTrackTimeBased_ToMatch, locDetectorMatches, locBCALShowerMatchParams))
		{
			const DBCALShower* locBCALShower = locBCALShowerMatchParams->dBCALShower;
			locTrackTimeBased->setTime(locBCALShower->t - locBCALShowerMatchParams->dFlightTime);
			(*locCovarianceMatrix)(6,6) = 0.25*0.25+locBCALShowerMatchParams->dFlightTimeVariance;
		}
		// TOF
		else if(locPIDAlgorithm->Get_BestTOFMatchParams(locTrackTimeBased_ToMatch, locDetectorMatches, locTOFHitMatchParams))
		{
			locTrackTimeBased->setTime(locTOFHitMatchParams->dHitTime - locTOFHitMatchParams->dFlightTime);
			(*locCovarianceMatrix)(6,6) = 0.1*0.1+locTOFHitMatchParams->dFlightTimeVariance;
		}
		// FCAL
		else if(locPIDAlgorithm->Get_BestFCALMatchParams(locTrackTimeBased_ToMatch, locDetectorMatches, locFCALShowerMatchParams))
		{
			const DFCALShower* locFCALShower = locFCALShowerMatchParams->dFCALShower;
			locTrackTimeBased->setTime(locFCALShower->getTime() - locFCALShowerMatchParams->dFlightTime);
			(*locCovarianceMatrix)(6,6) = 0.7*0.7+locFCALShowerMatchParams->dFlightTimeVariance;
		}
		// Start Counter
		else if(locPIDAlgorithm->Get_BestSCMatchParams(locTrackTimeBased_ToMatch, locDetectorMatches, locSCHitMatchParams))
		{
			double locPropagatedTime = locSCHitMatchParams->dHitTime - locSCHitMatchParams->dFlightTime;
			locTrackTimeBased->setTime(locPropagatedTime);
			(*locCovarianceMatrix)(6,6) = 0.3*0.3+locSCHitMatchParams->dFlightTimeVariance;
		}
	}
	
	locTrackTimeBased->setErrorMatrix(locCovarianceMatrix);
}


jerror_t DVertex_KLong_factory::Create_Vertex_NoTracks()
{
	// for now, assume 64 ns beam bunch spacing - will get this from CCDB later
	// if we don't know the time, arbitrarily guess that it's half the bunch spacing. 
	// could also guess that it is something unrealistic like 0. or the full bunch spacing - something to think about
	double locHalfBunchSpacing = 64./2.;   

	DVertex* locVertex = new DVertex();
	locVertex->dSpacetimeVertex = DLorentzVector(DVector3(m_beamSpotX, m_beamSpotY, dTargetZCenter), locHalfBunchSpacing);
	locVertex->dKinFitNDF = 0;
	locVertex->dKinFitChiSq = 0.0;

	//error matrix
	locVertex->dCovarianceMatrix.ResizeTo(4, 4);
	locVertex->dCovarianceMatrix.Zero();
	locVertex->dCovarianceMatrix(0, 0) = dTargetRadius*dTargetRadius/12.0; //x variance //should instead use beam spot size
	locVertex->dCovarianceMatrix(1, 1) = dTargetRadius*dTargetRadius/12.0; //y variance //should instead use beam spot size
	locVertex->dCovarianceMatrix(2, 2) = dTargetLength*dTargetLength/12.0; //z variance
	locVertex->dCovarianceMatrix(3, 3) = locHalfBunchSpacing; //t variance - we basically know nothing without a track

	Insert(locVertex);
	return NOERROR;
}

jerror_t DVertex_KLong_factory::Create_Vertex_OneTrack(const DTrackTimeBased* locTrackTimeBased)
{
	DVector3 locPosition = locTrackTimeBased->position();
	double locTime = locTrackTimeBased->time();

	DVertex* locVertex = new DVertex();
	locVertex->dSpacetimeVertex = DLorentzVector(locPosition, locTime);
	locVertex->dKinFitNDF = 0;
	locVertex->dKinFitChiSq = 0.0;

	//error matrix
	if(locTrackTimeBased->errorMatrix() != nullptr)
	{
		const TMatrixFSym& locTrackErrorMatrix = *(locTrackTimeBased->errorMatrix());
		locVertex->dCovarianceMatrix.ResizeTo(4, 4);
		locVertex->dCovarianceMatrix.Zero();
		for(size_t loc_i = 0; loc_i < 4; ++loc_i)
		{
			for(size_t loc_j = 0; loc_j < 4; ++loc_j)
				locVertex->dCovarianceMatrix(loc_i, loc_j) = locTrackErrorMatrix(loc_i + 3, loc_j + 3);
		}
	}

	Insert(locVertex);
	return NOERROR;
}

jerror_t DVertex_KLong_factory::Create_Vertex_Rough(DVector3 locPosition, double locTime)
{
	DVertex* locVertex = new DVertex();
	locVertex->dSpacetimeVertex = DLorentzVector(locPosition, locTime);
	locVertex->dKinFitNDF = 0;
	locVertex->dKinFitChiSq = 0.0;

	//error matrix //too lazy to compute properly right now ... need to hack DAnalysisUtilities::Calc_DOCA()
	locVertex->dCovarianceMatrix.ResizeTo(4, 4);
	locVertex->dCovarianceMatrix.Zero();
	locVertex->dCovarianceMatrix(0, 0) = 0.65; //x variance //from monitoring plots of vertex
	locVertex->dCovarianceMatrix(1, 1) = 0.65; //y variance //from monitoring plots of vertex
	locVertex->dCovarianceMatrix(2, 2) = 1.5; //z variance //a guess, semi-guarding against the worst case scenario //ugh
	locVertex->dCovarianceMatrix(3, 3) = 1.; //t variance - FIX

	Insert(locVertex);
	return NOERROR;
}

jerror_t DVertex_KLong_factory::Create_Vertex_KinFit(vector<const DKinematicData*> &locKinematicDataVector)
{
	auto locResultVertexConstraint = std::dynamic_pointer_cast<DKinFitConstraint_Vertex>(*dKinFitter->Get_KinFitConstraints().begin());

	TVector3 locFitVertex = locResultVertexConstraint->Get_CommonVertex();
	DVector3 locDFitVertex(locFitVertex.X(), locFitVertex.Y(), locFitVertex.Z());
	double locTime = 0.;
	if(dUseWeightedAverage)
		locTime = dAnalysisUtilities->Calc_CrudeWeightedTime(locKinematicDataVector, const_cast<DVector3&>(locDFitVertex));
	else
		locTime = dAnalysisUtilities->Calc_CrudeTime(locKinematicDataVector, const_cast<DVector3&>(locDFitVertex));

	DVertex* locVertex = new DVertex();
	locVertex->dSpacetimeVertex = DLorentzVector(locDFitVertex, locTime);
	locVertex->dKinFitNDF = dKinFitter->Get_NDF();
	locVertex->dKinFitChiSq = dKinFitter->Get_ChiSq();

	//error matrix
	const TMatrixDSym& locMatrixDSym = dKinFitter->Get_VXi();
	locVertex->dCovarianceMatrix.ResizeTo(4, 4);
	locVertex->dCovarianceMatrix.Zero();
	for(size_t loc_i = 0; loc_i < 3; ++loc_i)
	{
		for(size_t loc_j = 0; loc_j < 3; ++loc_j)
			locVertex->dCovarianceMatrix(loc_i, loc_j) = locMatrixDSym(loc_i, loc_j);
	}
	locVertex->dCovarianceMatrix(3, 3) = 1.; //t variance - FIX

	//Particle Maps & Pulls
	//Build pulls from this:
	map<shared_ptr<DKinFitParticle>, map<DKinFitPullType, double> > locPulls_KinFitParticle;
	dKinFitter->Get_Pulls(locPulls_KinFitParticle);

	//By looping over the pulls:
	auto locMapIterator = locPulls_KinFitParticle.begin();
	for(; locMapIterator != locPulls_KinFitParticle.end(); ++locMapIterator)
	{
		auto locOutputKinFitParticle = locMapIterator->first;
		auto locInputKinFitParticle = dKinFitUtils->Get_InputKinFitParticle(locOutputKinFitParticle);

		const JObject* locSourceJObject = dKinFitUtils->Get_SourceJObject(locInputKinFitParticle);
		locVertex->dKinFitPulls[locSourceJObject] = locMapIterator->second;
	}

	Insert(locVertex);

	return NOERROR;
}
