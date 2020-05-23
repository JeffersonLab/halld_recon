// $Id$
//
//    File: DReaction_factory_dirc_reactions.cc
//

#include "DReaction_factory_dirc_reactions.h"

//------------------
// brun
//------------------
jerror_t DReaction_factory_dirc_reactions::brun(JEventLoop* locEventLoop, int32_t locRunNumber)
{
	return NOERROR;
}

//------------------
// init
//------------------
jerror_t DReaction_factory_dirc_reactions::evnt(JEventLoop* locEventLoop, uint64_t locEventNumber)
{
	// Make as many DReaction objects as desired
	DReactionStep* locReactionStep = NULL;

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software
	// DReaction factory: https://halldweb1.jlab.org/wiki/index.php/Analysis_DReaction

	/**************************************************** p2pi_dirc Reaction Steps ****************************************************/

	DReaction* locReaction = new DReaction("p2pi_dirc"); //needs to be a unique name for each DReaction object, CANNOT (!) be "Thrown"

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

	locReaction->Enable_TTreeOutput("tree_p2pi_dirc.root");

	/**************************************************** p2pi_dirc Analysis Actions ****************************************************/

	// Recommended: Analysis actions automatically performed by the DAnalysisResults factories to histogram useful quantities.
	//These actions are executed sequentially, and are executed on each surviving (non-cut) particle combination
	//Pre-defined actions can be found in ANALYSIS/DHistogramActions.h and ANALYSIS/DCutActions.h
	
	// PID
	locReaction->Add_AnalysisAction(new DHistogramAction_PID(locReaction, false));
	
	//MASSES
	deque<Particle_t> locRhoPIDs;  locRhoPIDs.push_back(PiPlus);  locRhoPIDs.push_back(PiMinus);
	locReaction->Add_AnalysisAction(new DHistogramAction_MissingMassSquared(locReaction, false, 1000, -0.1, 0.1));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locRhoPIDs, false, 900, 0.3, 1.2, "Rho"));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locRhoPIDs, true, 900, 0.3, 1.2, "Rho_KinFit"));

	// Kinematic Fit Results
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05, true)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, 1e-5));

	// MASSES, POST-KINFIT
	locReaction->Add_AnalysisAction(new DHistogramAction_MissingMassSquared(locReaction, false, 1000, -0.1, 0.1, "PostKinFitCut"));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locRhoPIDs, false, 900, 0.3, 1.2, "Rho_PostKinFitCut"));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locRhoPIDs, true, 900, 0.3, 1.2, "Rho_KinFit_PostKinFitCut"));
	locReaction->Add_AnalysisAction(new DCutAction_InvariantMass(locReaction, 0, locRhoPIDs, true, 0.6, 0.9));

	// Custom histograms for DIRC
	locReaction->Add_AnalysisAction(new DCustomAction_dirc_track_pair(locReaction, true, 0, 1, 2, "PairPion_DIRC_KinFit"));
	locReaction->Add_AnalysisAction(new DCustomAction_dirc_reactions(locReaction, true, PiPlus, 0, 1, "PiPlus_DIRC_KinFit"));
	locReaction->Add_AnalysisAction(new DCustomAction_dirc_reactions(locReaction, true, PiMinus, 0, 2, "PiMinus_DIRC_KinFit"));

	// Kinematics of final selection
	locReaction->Add_AnalysisAction(new DHistogramAction_ParticleComboKinematics(locReaction, false)); //false: fill histograms with measured particle data

	_data.push_back(locReaction); //Register the DReaction with the factory

	
	/**************************************************** p2k_dirc Reaction Steps ****************************************************/

	locReaction = new DReaction("p2k_dirc"); //needs to be a unique name for each DReaction object, CANNOT (!) be "Thrown"

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

	locReaction->Enable_TTreeOutput("tree_p2k_dirc.root");

	/**************************************************** p2pi_dirc Analysis Actions ****************************************************/

	// Recommended: Analysis actions automatically performed by the DAnalysisResults factories to histogram useful quantities.
	//These actions are executed sequentially, and are executed on each surviving (non-cut) particle combination
	//Pre-defined actions can be found in ANALYSIS/DHistogramActions.h and ANALYSIS/DCutActions.h
	
	// PID
	locReaction->Add_AnalysisAction(new DHistogramAction_PID(locReaction, false));
	
	//MASSES
	deque<Particle_t> locPhiPIDs;  locPhiPIDs.push_back(KPlus);  locPhiPIDs.push_back(KMinus);
	locReaction->Add_AnalysisAction(new DHistogramAction_MissingMassSquared(locReaction, false, 1000, -0.1, 0.1));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locPhiPIDs, false, 500, 0.9, 1.4, "Phi"));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locPhiPIDs, true, 500, 0.9, 1.4, "Phi_KinFit"));

	// Kinematic Fit Results
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05, true)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, 1e-5)); 

	// MASSES, POST-KINFIT
	locReaction->Add_AnalysisAction(new DHistogramAction_MissingMassSquared(locReaction, false, 1000, -0.1, 0.1, "PostKinFitCut"));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locPhiPIDs, false, 500, 0.9, 1.4, "Phi_PostKinFitCut"));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locPhiPIDs, true, 500, 0.9, 1.4, "Phi_KinFit_PostKinFitCut"));
	locReaction->Add_AnalysisAction(new DCutAction_InvariantMass(locReaction, 0, locPhiPIDs, true, 1.0, 1.04));

	// Custom histograms for DIRC
	locReaction->Add_AnalysisAction(new DCustomAction_dirc_track_pair(locReaction, true, 0, 1, 2, "PairKaon_DIRC_KinFit"));
	locReaction->Add_AnalysisAction(new DCustomAction_dirc_reactions(locReaction, true, KPlus, 0, 1, "KPlus_DIRC_KinFit"));
	locReaction->Add_AnalysisAction(new DCustomAction_dirc_reactions(locReaction, true, KMinus, 0, 2, "KMinus_DIRC_KinFit"));

	// Kinematics of final selection
	locReaction->Add_AnalysisAction(new DHistogramAction_ParticleComboKinematics(locReaction, false)); //false: fill histograms with measured particle data

	_data.push_back(locReaction); //Register the DReaction with the factory

	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DReaction_factory_dirc_reactions::fini(void)
{
	for(size_t loc_i = 0; loc_i < dReactionStepPool.size(); ++loc_i)
		delete dReactionStepPool[loc_i]; //cleanup memory
	return NOERROR;
}

