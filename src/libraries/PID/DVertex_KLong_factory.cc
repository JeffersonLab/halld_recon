// $Id$
//
//    File: DVertex_KLong_factory.cc
// Created: Tue Apr  6 17:01:54 EDT 2010
// Creator: davidl (on Darwin Amelia.local 9.8.0 i386)
//

#include "DVertex_KLong_factory.h"
#include "PID/DChargedTrack.h"
#include "PID/DChargedTrackHypothesis.h"

//------------------
// init
//------------------
jerror_t DVertex_KLong_factory::init(void)
{
	dKinFitDebugLevel = 0;
	dMinTrackingFOM = 5.73303E-7;
	dNoKinematicFitFlag = false;
	dForceTargetCenter = false;
	dUseWeightedAverage = false;
	dMinTrackingNDF = 7;
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DVertex_KLong_factory::brun(jana::JEventLoop* locEventLoop, int32_t runnumber)
{
	dKinFitUtils = new DKinFitUtils_GlueX(locEventLoop);
	dKinFitter = new DKinFitter(dKinFitUtils);
	dKinFitUtils->Set_UpdateCovarianceMatricesFlag(false);

	// Get Target parameters from XML
	dTargetZCenter = 65.0;
	dTargetLength = 30.0;
	dTargetRadius = 1.5; //FIX: grab from database!!!
	m_beamSpotX = 0;
	m_beamSpotY = 0;
	DApplication* locApplication = dynamic_cast<DApplication*>(locEventLoop->GetJApplication());
	DGeometry* locGeometry = locApplication->GetDGeometry(locEventLoop->GetJEvent().GetRunNumber());
	locGeometry->GetTargetZ(dTargetZCenter);
	locGeometry->GetTargetLength(dTargetLength);
	jana::JCalibration *jcalib = japp->GetJCalibration(locEventLoop->GetJEvent().GetRunNumber());
	std::map<string, float> beam_spot;
	jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
	m_beamSpotX = beam_spot.at("x");
	m_beamSpotY = beam_spot.at("y");

	gPARMS->SetDefaultParameter("VERTEX:USE_TARGET_CENTER", dForceTargetCenter);
	gPARMS->SetDefaultParameter("VERTEX:NO_KINFIT_FLAG", dNoKinematicFitFlag);
	gPARMS->SetDefaultParameter("VERTEX:DEBUGLEVEL", dKinFitDebugLevel);
	gPARMS->SetDefaultParameter("VERTEX:MINTRACKINGFOM", dMinTrackingFOM);
	gPARMS->SetDefaultParameter("VERTEX:MINTRACKNDF", dMinTrackingNDF);
	gPARMS->SetDefaultParameter("VERTEX:USEWEIGHTEDAVERAGE", dUseWeightedAverage);

	dKinFitter->Set_DebugLevel(dKinFitDebugLevel);

	locEventLoop->GetSingle(dAnalysisUtilities);

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DVertex_KLong_factory::evnt(JEventLoop* locEventLoop, uint64_t eventnumber)
{
	// For KLong reconstruction, the beam is massive, so we figure out the vertex time based on
	// the reconstructed tracks, and figure out the KLong momentum from this

	//preferentially (kinematic fit):
		//use tracks with a matched hit & good tracking FOM
		//if no good tracks (or none with matched hits), use all tracks
		//if no tracks, use target center

// 	vector<const DTrackTimeBased*> locTrackTimeBasedVector;
// 	locEventLoop->Get(locTrackTimeBasedVector);

	vector<const DChargedTrack*> locChargedTrackVector;
	locEventLoop->Get(locChargedTrackVector);

	const DDetectorMatches* locDetectorMatches = NULL;
	locEventLoop->GetSingle(locDetectorMatches);

	// give option for just using the target center, e.g. if the magnetic
	// field is off and/or tracking is otherwise not working well
	if(dForceTargetCenter)
		return Create_Vertex_NoTracks();

// 	select the best DTrackTimeBased for each track: use best tracking FOM
// 	map<JObject::oid_t, const DTrackTimeBased*> locBestTrackTimeBasedMap; //lowest tracking chisq/ndf for each candidate id
// 	for(size_t loc_i = 0; loc_i < locTrackTimeBasedVector.size(); ++loc_i)
// 	{
// 		JObject::oid_t locCandidateID = locTrackTimeBasedVector[loc_i]->candidateid;
// 		if(locBestTrackTimeBasedMap.find(locCandidateID) == locBestTrackTimeBasedMap.end())
// 			locBestTrackTimeBasedMap[locCandidateID] = locTrackTimeBasedVector[loc_i];
// 		else if(locTrackTimeBasedVector[loc_i]->FOM > locBestTrackTimeBasedMap[locCandidateID]->FOM)
// 			locBestTrackTimeBasedMap[locCandidateID] = locTrackTimeBasedVector[loc_i];
// 	}

	//separate the tracks based on high/low tracking FOM & has hit-match
	//map<JObject::oid_t, const DTrackTimeBased*>::iterator locIterator;
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

	vector<const DTrackTimeBased*> locTrackTimeBasedVectorToUse = (locTrackTimeBasedVector_OnePerTrack_Good.size() >= 2) ? locTrackTimeBasedVector_OnePerTrack_Good : locTrackTimeBasedVector_OnePerTrack;
	vector<const DKinematicData*> locKinematicDataVectorToUse = (locKinematicDataVector_OnePerTrack_Good.size() >= 2) ? locKinematicDataVector_OnePerTrack : locKinematicDataVector_OnePerTrack;

	//handle cases of no/one track
	if(locTrackTimeBasedVectorToUse.empty())
		return Create_Vertex_NoTracks();
	if(locTrackTimeBasedVectorToUse.size() == 1)
		return Create_Vertex_OneTrack(locTrackTimeBasedVectorToUse[0]);

	// first calculate a rough vertex
	DVector3 locRoughPosition = dAnalysisUtilities->Calc_CrudeVertex(locTrackTimeBasedVectorToUse);
	double locRoughTime = 0.;
	if(dUseWeightedAverage)
		locRoughTime = dAnalysisUtilities->Calc_CrudeWeightedTime(locKinematicDataVectorToUse, const_cast<DVector3&>(locRoughPosition));
	else
		locRoughTime = dAnalysisUtilities->Calc_CrudeTime(locKinematicDataVectorToUse, const_cast<DVector3&>(locRoughPosition));

	// if only want rough guess, save it and exit
	if(dNoKinematicFitFlag || (locTrackTimeBasedVectorToUse[0]->errorMatrix() == nullptr))
		return Create_Vertex_Rough(locRoughPosition, locRoughTime);

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
	if(!dKinFitter->Fit_Reaction()) //if fit fails to converge: use rough results
		return Create_Vertex_Rough(locRoughPosition, locRoughTime);

	//save kinfit results
	return Create_Vertex_KinFit(locKinematicDataVectorToUse);
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

	_data.push_back(locVertex);
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

	_data.push_back(locVertex);
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

	_data.push_back(locVertex);
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

	_data.push_back(locVertex);

	return NOERROR;
}
