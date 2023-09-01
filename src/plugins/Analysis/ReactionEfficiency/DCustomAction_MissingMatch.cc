// $Id$
//
//    File: DCustomAction_MissingMatch.cc
// Created: Wed Jun 19 17:20:17 EDT 2019
// Creator: jrsteven (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "DCustomAction_MissingMatch.h"

void DCustomAction_MissingMatch::Initialize(JEventLoop* locEventLoop)
{
	//Optional: Create histograms and/or modify member variables.
	//Create any histograms/trees/etc. within a ROOT lock. 
		//This is so that when running multithreaded, only one thread is writing to the ROOT file at a time. 
	//NEVER: Get anything from the JEventLoop while in a lock: May deadlock

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();

		dHistMissingMatch2DTOF = GetOrCreate_Histogram<TH2F>("HistMissingMatch2DTOF", "HistMissingMatch2DTOF", 100, -10, 10, 100, 0.0, 100.0);
		dHistMissingMatch2DBCAL = GetOrCreate_Histogram<TH2F>("HistMissingMatch2DBCAL", "HistMissingMatch2DBCAL", 100, -10, 10, 100, 0.0, 100.0);

		dHistMissingMatchDistTOF = GetOrCreate_Histogram<TH1F>("HistMissingMatchDistTOF", "HistMissingMatchDistTOF", 100, 0.0, 100.0);
		dHistMissingMatchDistBCAL = GetOrCreate_Histogram<TH1F>("HistMissingMatchDistBCAL", "HistMissingMatchDistBCAL", 100, 0.0, 100.0);
		ChangeTo_BaseDirectory();
	}
	japp->RootUnLock(); //RELEASE ROOT LOCK!!

	// repeated in Run_Update() for possible CCDB updates each run(?)
        const DParticleID* locParticleID = NULL;
        locEventLoop->GetSingle(locParticleID);
        dParticleID = locParticleID;

	DApplication* dapp=dynamic_cast<DApplication*>(locEventLoop->GetJApplication());
	bfield = dapp->GetBfield((locEventLoop->GetJEvent()).GetRunNumber());
	rt = new DReferenceTrajectory(bfield);

	locEventLoop->GetSingle(dAnalysisUtilities);
}

bool DCustomAction_MissingMatch::Perform_Action(JEventLoop* locEventLoop, const DParticleCombo* locParticleCombo)
{       
	// distance of best match to TOF and BCAL
	double locBestMissingMatchDistTOF = 999;
	double locBestMissingMatchDistBCAL = 999;

	// get missing particle
	vector<const DKinematicData*> locMissingParticles = locParticleCombo->Get_MissingParticles(Get_Reaction());
	if(locMissingParticles.empty()) return false;

	const DKinematicData *locMissing = locMissingParticles[0];
        DVector3 locMissingPosition = locMissing->position();
	DVector3 locMissingMomentum = locMissing->momentum();
	double locInputStartTime = locParticleCombo->Get_EventVertex().T();

	rt->Reset();
	rt->Swim(locMissingPosition, locMissingMomentum, locMissing->charge());

	/*
	// try with MC track for PiPlus?
	vector<const DMCThrown*> locMCThrowns;
	locEventLoop->Get(locMCThrowns);
	DVector3 locMCThrownPosition;
	DVector3 locMCThrownMomentum;
	double locMCThrownTime;
	for(size_t loc_i=0; loc_i<locMCThrowns.size(); loc_i++) {
		if(locMCThrowns[loc_i]->PID() == locMissing->PID()) {
			locMCThrownPosition = locMCThrowns[loc_i]->position();
			locMCThrownMomentum = locMCThrowns[loc_i]->momentum();
			locMCThrownTime = locMCThrowns[loc_i]->time();
			break;
		}
	}
	
	locMissingPosition.Print();
	locMCThrownPosition.Print();
	locMissingMomentum.Print();
	locMCThrownMomentum.Print();
	cout<<locInputStartTime<<" "<<locMCThrownTime<<endl;

	cout<<"MCThrown"<<endl;
	locMCThrownMomentum.Print();

	rt->Swim(locMCThrownPosition, locMCThrownMomentum, locMissing->charge());
	*/

	// compute distance from unused BCAL showers
	vector<const DNeutralShower*> locUnusedNeutralShowers;
	dAnalysisUtilities->Get_UnusedNeutralShowers(locEventLoop, locParticleCombo, locUnusedNeutralShowers);
	for(size_t loc_i=0; loc_i<locUnusedNeutralShowers.size(); loc_i++) {
		if(locUnusedNeutralShowers[loc_i]->dDetectorSystem != SYS_BCAL) continue;
		
		const DBCALShower *locBCALShower;
		locUnusedNeutralShowers[loc_i]->GetSingle(locBCALShower);

		shared_ptr<DBCALShowerMatchParams>locShowerMatchParams;
		DVector3 locOutputProjPos, locOutputProjMom;
		if(dParticleID->Distance_ToTrack(rt, locBCALShower, locInputStartTime, locShowerMatchParams, &locOutputProjPos, &locOutputProjMom)) {

			double locDeltaT = locBCALShower->t - locShowerMatchParams->dFlightTime - locInputStartTime;
			double locMissingMatchDist = locShowerMatchParams->Get_DistanceToTrack();
			dHistMissingMatch2DBCAL->Fill(locDeltaT, locMissingMatchDist);

			if(fabs(locDeltaT) > 1.0) continue;

			// keep best matched BCAL hit
			if(locMissingMatchDist < locBestMissingMatchDistBCAL) 
				locBestMissingMatchDistBCAL = locMissingMatchDist;
		}
	}

	// Replace with Get_UnusedTOFPoints function to match custom DEventWriter...

	// compute distance from TOF
	vector<const DTOFPoint*> locTOFPoints;
	locEventLoop->Get(locTOFPoints);
	for(size_t loc_i=0; loc_i<locTOFPoints.size(); loc_i++) {
		
		// check if TOF point is used by another measured track
		bool locIsTOFPointUsed = false;
		for(size_t loc_j = 0; loc_j < dChargedIndices.size(); ++loc_j) {
			const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(0);
			auto locParticle = Get_UseKinFitResultsFlag() ? locParticleComboStep->Get_FinalParticle(dChargedIndices[loc_j]) : locParticleComboStep->Get_FinalParticle_Measured(dChargedIndices[loc_j]);
			const DChargedTrack* locChargedTrack = static_cast<const DChargedTrack*>(locParticleComboStep->Get_FinalParticle_SourceObject(dChargedIndices[loc_j]));
			const DChargedTrackHypothesis* locChargedTrackHypothesis = locChargedTrack->Get_Hypothesis(locParticle->PID());
			
			if(locChargedTrackHypothesis->Get_TOFHitMatchParams() == nullptr) continue;
			const DTOFPoint *locUsedTOFPoint = locChargedTrackHypothesis->Get_TOFHitMatchParams()->dTOFPoint;
			if(locUsedTOFPoint == locTOFPoints[loc_i]) {
				locIsTOFPointUsed = true;
				break;
			}
		}
		if(locIsTOFPointUsed) continue;

		shared_ptr<DTOFHitMatchParams>locHitMatchParams;
		DVector3 locOutputProjPos, locOutputProjMom;
		
		if(dParticleID->Distance_ToTrack(rt, locTOFPoints[loc_i], locInputStartTime, locHitMatchParams, &locOutputProjPos, &locOutputProjMom)) {

			double locDeltaT = locHitMatchParams->dHitTime - locHitMatchParams->dFlightTime - locInputStartTime;
			double locMissingMatchDist = locHitMatchParams->Get_DistanceToTrack();
			dHistMissingMatch2DTOF->Fill(locDeltaT, locMissingMatchDist);

			if(fabs(locDeltaT) > 0.3) continue;

			// keep best matched TOF hit
			if(locMissingMatchDist < locBestMissingMatchDistTOF) 
				locBestMissingMatchDistTOF = locMissingMatchDist;
		}
	}
	
	Lock_Action(); //ACQUIRE ROOT LOCK!!
	{
		dHistMissingMatchDistTOF->Fill(locBestMissingMatchDistTOF);
		dHistMissingMatchDistBCAL->Fill(locBestMissingMatchDistBCAL);
	}
	Unlock_Action(); //RELEASE ROOT LOCK!!

	return true; //return false if you want to use this action to apply a cut (and it fails the cut!)
}
