// $Id$
//
//    File: DCustomAction_RecoilMass.cc
// Created: Wed Jun 19 17:20:17 EDT 2019
// Creator: jrsteven (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "DCustomAction_RecoilMass.h"

void DCustomAction_RecoilMass::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	//Optional: Create histograms and/or modify member variables.
	//Create any histograms/trees/etc. within a ROOT lock. 
		//This is so that when running multithreaded, only one thread is writing to the ROOT file at a time. 
	//NEVER: Get anything from the JEventLoop while in a lock: May deadlock

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();
		dHistRecoilMass = GetOrCreate_Histogram<TH1F>("HistRecoilMass", "HistRecoilMass", 100, 0.0, 2.0);
		ChangeTo_BaseDirectory();
	}
	GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

bool DCustomAction_RecoilMass::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{       
	// get beam, target and final state recoil particle to subtract
	const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(0);
	auto locBeamPhoton = Get_UseKinFitResultsFlag() ? locParticleComboStep->Get_InitialParticle() : locParticleComboStep->Get_InitialParticle_Measured();
	TLorentzVector locRecoilSumP4;
	
	for(size_t i=0; i<dRecoilIndices.size(); i++) {
		auto locParticle = Get_UseKinFitResultsFlag() ? locParticleComboStep->Get_FinalParticle(dRecoilIndices[i]) : locParticleComboStep->Get_FinalParticle_Measured(dRecoilIndices[i]);
		locRecoilSumP4 += locParticle->lorentzMomentum();
	}
	
	TLorentzVector locTargetP4(0,0,0,0.938);
	TLorentzVector locRecoilP4 = locBeamPhoton->lorentzMomentum() + locTargetP4;
	locRecoilP4 -= locRecoilSumP4;
	double locRecoilMass = locRecoilP4.M();

	Lock_Action(); //ACQUIRE ROOT LOCK!!
	{
		dHistRecoilMass->Fill(locRecoilMass);
	}
	Unlock_Action(); //RELEASE ROOT LOCK!!

	if(locRecoilMass < dMinRecoilMass || locRecoilMass > dMaxRecoilMass)
		return false;

	return true; //return false if you want to use this action to apply a cut (and it fails the cut!)
}
