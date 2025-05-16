// $Id$
//
//    File: DCustomAction_p3pi_Pi0Cuts_FCAL2.cc
// Created: Thu Jan 22 11:19:47 EST 2015
// Creator: jrsteven (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#include "DCustomAction_p3pi_Pi0Cuts_FCAL2.h"

void DCustomAction_p3pi_Pi0Cuts_FCAL2::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	
}

bool DCustomAction_p3pi_Pi0Cuts_FCAL2::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(2);
        if(Get_Reaction()->Get_ReactionStep(2)->Get_InitialPID() != Pi0)
                return false;
	
	// get final state particles
   	auto locParticles = Get_UseKinFitResultsFlag() ? locParticleComboStep->Get_FinalParticles() : locParticleComboStep->Get_FinalParticles_Measured();

	int nFECAL = 0;  // FCAL2 = FCAL + ECAL
	
        // loop final state particles
        for(size_t loc_i = 0; loc_i < locParticles.size(); ++loc_i) {
               if(locParticles[loc_i] == nullptr) continue;
		
		// get shower object
		const DNeutralShower* locNeutralShower = static_cast<const DNeutralShower*>(locParticleComboStep->Get_FinalParticle_SourceObject(loc_i));
		if(locNeutralShower == NULL) 
			continue;
		
		// count # of FCAL photons and set separate thresholds on FCAL and BCAL energies
		if(locNeutralShower->dDetectorSystem == SYS_FCAL) {
			nFECAL++;
		}
		if(locNeutralShower->dDetectorSystem == SYS_ECAL) {
			nFECAL++;
		}
	}

	// require 1 or 2 photons in FCAL2 (specify parameter in DReaction setup)
	if(nFECAL != dMinFECAL) 
		return false;

	// passed all cuts
	return true; 
}
