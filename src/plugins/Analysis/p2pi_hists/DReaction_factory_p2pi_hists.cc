// $Id$
//
//    File: DReaction_factory_p2pi_hists.cc
// Created: Wed Mar 11 20:34:14 EDT 2015
// Creator: jrsteven (on Linux halldw1.jlab.org 2.6.32-504.8.1.el6.x86_64 x86_64)
//

#include "DReaction_factory_p2pi_hists.h"
#include "DANA/DEvent.h"


//------------------
// BeginRun
//------------------
void DReaction_factory_p2pi_hists::BeginRun(const std::shared_ptr<const JEvent> &locEvent)
{
	vector<double> locBeamPeriodVector;
	DEvent::GetCalib(locEvent, "PHOTON_BEAM/RF/beam_period", locBeamPeriodVector);
	dBeamBunchPeriod = locBeamPeriodVector[0];
}

//------------------
// Init
//------------------
void DReaction_factory_p2pi_hists::Process(const std::shared_ptr<const JEvent> &locEvent)
{
	// Make as many DReaction objects as desired
	DReactionStep* locReactionStep = NULL;
	DReaction* locReaction = new DReaction("p2pi_pmiss"); //needs to be a unique name for each DReaction object, CANNOT (!) be "Thrown"

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software
	// DReaction factory: https://halldweb1.jlab.org/wiki/index.php/Analysis_DReaction

	/**************************************************** p2pi_preco Reaction Steps ****************************************************/

	bool unused = false;
	locReaction = new DReaction("p2pi_preco"); //needs to be a unique name for each DReaction object, CANNOT (!) be "Thrown"

	// g, p -> pi+, pi- ,p
	locReactionStep = new DReactionStep();
	locReactionStep->Set_InitialParticleID(Gamma);
	locReactionStep->Set_TargetParticleID(Proton);
	locReactionStep->Add_FinalParticleID(Proton);
	locReactionStep->Add_FinalParticleID(PiPlus);
	locReactionStep->Add_FinalParticleID(PiMinus);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	/**************************************************** p2pi_preco Control Settings ****************************************************/

	// Event Store
	locReaction->Set_EventStoreSkims("2q+,q-"); // boolean-AND of skims

	// Kinematic Fit
	//locReaction->Set_KinFitType(d_NoFit); //simultaneously constrain apply four-momentum conservation, invariant masses, and common-vertex constraints

	// Highly Recommended: When generating particle combinations, reject all beam photons that match to a different RF bunch
	locReaction->Set_NumPlusMinusRFBunches(0);

	/**************************************************** p2pi_preco Analysis Actions ****************************************************/

	// Recommended: Analysis actions automatically performed by the DAnalysisResults factories to histogram useful quantities.
	//These actions are executed sequentially, and are executed on each surviving (non-cut) particle combination
	//Pre-defined actions can be found in ANALYSIS/DHistogramActions.h and ANALYSIS/DCutActions.h
	
	// PID
	locReaction->Add_AnalysisAction(new DHistogramAction_PID(locReaction, false));

	// Custom histograms for p2pi
	locReaction->Add_AnalysisAction(new DCustomAction_p2pi_hists(locReaction, false));

	//MISSING MASS
	locReaction->Add_AnalysisAction(new DHistogramAction_MissingMassSquared(locReaction, false, 1000, -0.1, 0.1));
	locReaction->Add_AnalysisAction(new DCutAction_MissingMassSquared(locReaction, false, -0.01, 0.005));

	// RHO
	deque<Particle_t> locRhoPIDs;  locRhoPIDs.push_back(PiPlus);  locRhoPIDs.push_back(PiMinus);
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locRhoPIDs, false, 900, 0.3, 1.2, "Rho"));

	if(unused)
	{
		// Custom cuts (can be applied in TSelector)
		locReaction->Add_AnalysisAction(new DCustomAction_p2pi_cuts(locReaction, false));

		// Diagnostics for unused tracks and showers with final selection (only useful when analyzing EVIO data)
		locReaction->Add_AnalysisAction(new DCustomAction_p2pi_unusedHists(locReaction, false, "Unused"));
	}

	// Kinematics of final selection
	locReaction->Add_AnalysisAction(new DHistogramAction_ParticleComboKinematics(locReaction, false)); //false: fill histograms with measured particle data

	Insert(locReaction); //Register the DReaction with the factory



	/**************************************************** p2pi_preco_kinfit Reaction Steps ****************************************************/

	locReaction = new DReaction("p2pi_preco_kinfit"); //needs to be a unique name for each DReaction object, CANNOT (!) be "Thrown"

	// g, p -> pi+, pi- ,p
	locReaction->Add_ReactionStep(locReactionStep);

	/**************************************************** p2pi_preco_kinfit Control Settings ****************************************************/

	// Event Store
	locReaction->Set_EventStoreSkims("2q+,q-"); // boolean-AND of skims

	// Kinematic Fit
	locReaction->Set_KinFitType(d_P4AndVertexFit); //simultaneously constrain apply four-momentum conservation, invariant masses, and common-vertex constraints

	// Highly Recommended: When generating particle combinations, reject all beam photons that match to a different RF bunch
	locReaction->Set_NumPlusMinusRFBunches(0);

	/**************************************************** p2pi_preco_kinfit Analysis Actions ****************************************************/

	// Recommended: Analysis actions automatically performed by the DAnalysisResults factories to histogram useful quantities.
	//These actions are executed sequentially, and are executed on each surviving (non-cut) particle combination
	//Pre-defined actions can be found in ANALYSIS/DHistogramActions.h and ANALYSIS/DCutActions.h

	// PID
	locReaction->Add_AnalysisAction(new DHistogramAction_PID(locReaction, false));

	//MASSES
	locReaction->Add_AnalysisAction(new DHistogramAction_MissingMassSquared(locReaction, false, 1000, -0.1, 0.1));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locRhoPIDs, false, 900, 0.3, 1.2, "Rho"));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locRhoPIDs, true, 900, 0.3, 1.2, "Rho_KinFit"));

	// Kinematic Fit Results
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05, true)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, 5.73303E-7)); // confidence level cut //+/- 5 sigma

	// MASSES, POST-KINFIT
	locReaction->Add_AnalysisAction(new DHistogramAction_MissingMassSquared(locReaction, false, 1000, -0.1, 0.1, "PostKinFitCut"));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locRhoPIDs, false, 900, 0.3, 1.2, "Rho_PostKinFitCut"));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locRhoPIDs, true, 900, 0.3, 1.2, "Rho_KinFit_PostKinFitCut"));

	// Kinematics of final selection
	locReaction->Add_AnalysisAction(new DHistogramAction_ParticleComboKinematics(locReaction, false)); //false: fill histograms with measured particle data

	Insert(locReaction); //Register the DReaction with the factory
}

//------------------
// Finish
//------------------
void DReaction_factory_p2pi_hists::Finish()
{
	for(size_t loc_i = 0; loc_i < dReactionStepPool.size(); ++loc_i)
		delete dReactionStepPool[loc_i]; //cleanup memory
}

