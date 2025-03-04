// $Id$
//
//    File: DCustomAction_dirc_track_pair.cc
//

#include "DCustomAction_dirc_track_pair.h"

void DCustomAction_dirc_track_pair::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	// get PID algos
	const DParticleID* locParticleID = NULL;
	locEvent->GetSingle(locParticleID);
	dParticleID = locParticleID;

	// get DIRC geometry
	vector<const DDIRCGeometry*> locDIRCGeometry;
	locEvent->Get(locDIRCGeometry);
	dDIRCGeometry = locDIRCGeometry[0];
}

void DCustomAction_dirc_track_pair::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
        Run_Update(locEvent);

	//Optional: Create histograms and/or modify member variables.
	//Create any histograms/trees/etc. within a ROOT lock. 
		//This is so that when running multithreaded, only one thread is writing to the ROOT file at a time. 
	//NEVER: Get anything from the JEventLoop while in a lock: May deadlock

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();

		hComboCounter = GetOrCreate_Histogram<TH1I>("ComboCounter", "ComboCounter", 10, 0, 10);
		hDeltaBarX_SameBox = GetOrCreate_Histogram<TH2I>("DeltaBarX", "DeltaBarX", 100, -150, 150.0, 40, -20, 20);

		ChangeTo_BaseDirectory();
	}
	GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

bool DCustomAction_dirc_track_pair::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
        const DDetectorMatches* locDetectorMatches = NULL;
        locEvent->GetSingle(locDetectorMatches);

        // Get selected particle from reaction for DIRC analysis
	const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(dParticleComboStepIndex);
	auto locParticle1 = Get_UseKinFitResultsFlag() ? locParticleComboStep->Get_FinalParticle(dParticleIndex1) : locParticleComboStep->Get_FinalParticle_Measured(dParticleIndex1);
	auto locParticle2 = Get_UseKinFitResultsFlag() ? locParticleComboStep->Get_FinalParticle(dParticleIndex2) : locParticleComboStep->Get_FinalParticle_Measured(dParticleIndex2);
	
	const DChargedTrack* locChargedTrack1 = static_cast<const DChargedTrack*>(locParticleComboStep->Get_FinalParticle_SourceObject(dParticleIndex1));
	const DChargedTrack* locChargedTrack2 = static_cast<const DChargedTrack*>(locParticleComboStep->Get_FinalParticle_SourceObject(dParticleIndex2));
	const DChargedTrackHypothesis* locChargedTrackHypothesis1 = locChargedTrack1->Get_Hypothesis(locParticle1->PID());
	const DChargedTrackHypothesis* locChargedTrackHypothesis2 = locChargedTrack2->Get_Hypothesis(locParticle2->PID());
	const DTrackTimeBased* locTrackTimeBased1 = locChargedTrackHypothesis1->Get_TrackTimeBased();
	const DTrackTimeBased* locTrackTimeBased2 = locChargedTrackHypothesis2->Get_TrackTimeBased();

	// get DIRC match parameters (contains LUT information)
	shared_ptr<const DDIRCMatchParams> locDIRCMatchParams1;
	shared_ptr<const DDIRCMatchParams> locDIRCMatchParams2;
	bool foundDIRC1 = dParticleID->Get_DIRCMatchParams(locTrackTimeBased1, locDetectorMatches, locDIRCMatchParams1);
	bool foundDIRC2 = dParticleID->Get_DIRCMatchParams(locTrackTimeBased2, locDetectorMatches, locDIRCMatchParams2);

	hComboCounter->Fill(0);
	if(foundDIRC1 || foundDIRC2)
	  hComboCounter->Fill(1);

	// compare extrapolated track positions and angles of the pair...
	if(foundDIRC1 && !foundDIRC2) {
	  DVector3 posInBar = locDIRCMatchParams1->dExtrapolatedPos; 
	}
	if(foundDIRC2 && !foundDIRC1) {
	  DVector3 posInBar = locDIRCMatchParams2->dExtrapolatedPos; 
	}
	if(foundDIRC1 && foundDIRC2) {
	  hComboCounter->Fill(2);
	  DVector3 posInBar1 = locDIRCMatchParams1->dExtrapolatedPos; 
	  DVector3 posInBar2 = locDIRCMatchParams2->dExtrapolatedPos; 
	  DVector3 posDiff = posInBar1 - posInBar2;
	  
	  // both in same optical box
	  if((posInBar1.Y() > 0 && posInBar2.Y() > 0) || (posInBar1.Y() < 0 && posInBar2.Y() < 0)) {
	    hComboCounter->Fill(3);
	    int locDeltaBar = dDIRCGeometry->GetBar(posInBar1.Y()) - dDIRCGeometry->GetBar(posInBar2.Y());
	    hDeltaBarX_SameBox->Fill(posDiff.X(), locDeltaBar);
	  }
	  else {
	    //hDeltaBarX_DiffBox->Fill(posDiff.X(), locDeltaBar);
	  }
	}

	return true; //return false if you want to use this action to apply a cut (and it fails the cut!)
}
