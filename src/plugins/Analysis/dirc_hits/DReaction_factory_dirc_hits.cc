// $Id$
//
//    File: DReaction_factory_dirc_hits.cc
//

#include "DReaction_factory_dirc_hits.h"

//------------------
// brun
//------------------
jerror_t DReaction_factory_dirc_hits::brun(JEventLoop* locEventLoop, int32_t locRunNumber)
{
	return NOERROR;
}

//------------------
// init
//------------------
jerror_t DReaction_factory_dirc_hits::evnt(JEventLoop* locEventLoop, uint64_t locEventNumber)
{

        // Make as many DReaction objects as desired
	DReactionStep* locReactionStep = NULL;

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software
	// DReaction factory: https://halldweb1.jlab.org/wiki/index.php/Analysis_DReaction

	/**************************************************** p2pi_dirc Reaction Steps ****************************************************/

	DReaction* locReaction = new DReaction("p2pi_dirc_hits"); //needs to be a unique name for each DReaction object, CANNOT (!) be "Thrown"

	// g, p -> pi+, pi- ,p
	locReactionStep = new DReactionStep();
	locReactionStep->Set_InitialParticleID(Gamma);
	locReactionStep->Set_TargetParticleID(Proton);
	locReactionStep->Add_FinalParticleID(Proton);
	locReactionStep->Add_FinalParticleID(PiPlus);
	locReactionStep->Add_FinalParticleID(PiMinus);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	/**************************************************** p2pi_dirc Control Settings ****************************************************/

	// Kinematic Fit
	locReaction->Set_KinFitType(d_P4AndVertexFit); //simultaneously constrain apply four-momentum conservation, invariant masses, and common-vertex constraints
	
	// Highly Recommended: When generating particle combinations, reject all beam photons that match to a different RF bunch
	locReaction->Set_NumPlusMinusRFBunches(0);

	//locReaction->Enable_TTreeOutput("dtree_p2pi_dirc.root");

	/**************************************************** p2pi_dirc Analysis Actions ****************************************************/

	// Recommended: Analysis actions automatically performed by the DAnalysisResults factories to histogram useful quantities.
	//These actions are executed sequentially, and are executed on each surviving (non-cut) particle combination
	//Pre-defined actions can be found in ANALYSIS/DHistogramActions.h and ANALYSIS/DCutActions.h
	
	// PID
	// Kinematic Fit Results
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, 5.73303E-7)); // confidence level cut //+/- 5 sigma

	// Custom histograms for DIRC
	//locReaction->Add_AnalysisAction(new DCustomAction_dirc_hits(locReaction, true, PiPlus, 0, "PiPlus_DIRC_KinFit"));

	_data.push_back(locReaction); //Register the DReaction with the factory

	
	/**************************************************** p2pi_dirc Reaction Steps ****************************************************/

	locReaction = new DReaction("p2k_dirc_hits"); //needs to be a unique name for each DReaction object, CANNOT (!) be "Thrown"

	// g, p -> K+, K- ,p
	locReactionStep = new DReactionStep();
	locReactionStep->Set_InitialParticleID(Gamma);
	locReactionStep->Set_TargetParticleID(Proton);
	locReactionStep->Add_FinalParticleID(Proton);
	locReactionStep->Add_FinalParticleID(KPlus);
	locReactionStep->Add_FinalParticleID(KMinus);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	/**************************************************** p2pi_dirc Control Settings ****************************************************/

	// Kinematic Fit
	locReaction->Set_KinFitType(d_P4AndVertexFit); //simultaneously constrain apply four-momentum conservation, invariant masses, and common-vertex constraints
	
	// Highly Recommended: When generating particle combinations, reject all beam photons that match to a different RF bunch
	locReaction->Set_NumPlusMinusRFBunches(0);

	//locReaction->Enable_TTreeOutput("dtree_p2k_dirc.root");

	/**************************************************** p2pi_dirc Analysis Actions ****************************************************/

	// Recommended: Analysis actions automatically performed by the DAnalysisResults factories to histogram useful quantities.
	//These actions are executed sequentially, and are executed on each surviving (non-cut) particle combination
	//Pre-defined actions can be found in ANALYSIS/DHistogramActions.h and ANALYSIS/DCutActions.h
	
	// Kinematic Fit Results
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, 5.73303E-7)); // confidence level cut //+/- 5 sigma

	// Custom histograms for DIRC
	//locReaction->Add_AnalysisAction(new DCustomAction_dirc_hits(locReaction, true, KPlus, 0, "KPlus_DIRC_KinFit"));

	_data.push_back(locReaction); //Register the DReaction with the factory

	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DReaction_factory_dirc_hits::fini(void)
{
	for(size_t loc_i = 0; loc_i < dReactionStepPool.size(); ++loc_i)
		delete dReactionStepPool[loc_i]; //cleanup memory
	return NOERROR;
}

