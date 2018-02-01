// $Id$
//
//    File: DReaction_factory_p2k.cc
// Created: Thu Feb  1 10:49:03 EST 2018
// Creator: aebarnes (on Linux egbert 2.6.32-696.13.2.el6.x86_64 x86_64)
//


#include "DReaction_factory_p2k.h"

//------------------
// evnt
//------------------
jerror_t DReaction_factory_p2k::evnt(JEventLoop* locEventLoop, uint64_t locEventNumber)
{
	// Make as many DReaction objects as desired
	DReactionStep* locReactionStep = NULL;
	DReaction* locReaction = NULL; //create with a unique name for each DReaction object. CANNOT (!) be "Thrown"

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software
	// DReaction factory: https://halldweb1.jlab.org/wiki/index.php/Analysis_DReaction

	/************************************************** p2k Reaction Definition *************************************************/

	locReaction = new DReaction("p2k");

	/* 
	//Required: DReactionSteps to specify the channel and decay chain you want to study
		//Particles are of type Particle_t, an enum defined in sim-recon/src/libraries/include/particleType.h

	//Example: g, p -> pi+, pi-, pi0, (p)
	//Inputs: Beam, target, non-missing final-state particles (vector), missing final state particle (none by default), bool inclusive_flag = false by default
	locReactionStep = new DReactionStep(Gamma, Proton, {PiPlus, PiMinus, Pi0}, Proton);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	//Example: pi0 -> g, g
	//Inputs: Decaying, non-missing final-state particles (vector), missing final state particle (none by default), bool inclusive_flag = false by default
	locReactionStep = new DReactionStep(Pi0, {Gamma, Gamma});
	//locReactionStep->Set_KinFitConstrainInitMassFlag(false); //default: true //ignored if p4 not fit or is beam //phi, omega not constrained regardless
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak
	*/

	locReactionStep = new DReactionStep(Gamma, Proton, {KPlus, KMinus, Proton});
        locReaction->Add_ReactionStep(locReactionStep);
        dReactionStepPool.push_back(locReactionStep);

	/**************************************************** p2k Control Settings ****************************************************/

	// Highly Recommended: Set EventStore skim query (use with "eventstore" source)
		// This will skip creating particle combos for events that aren't in the skims you list
		// Query should be comma-separated list of skims to boolean-AND together
	//locReaction->Set_EventStoreSkims("myskim1,myskim2,myskim3"); //boolean-AND of skims

	// Recommended: Type of kinematic fit to perform (default is d_NoFit)
		//fit types are of type DKinFitType, an enum defined in sim-recon/src/libraries/ANALYSIS/DReaction.h
		//Options: d_NoFit (default), d_P4Fit, d_VertexFit, d_P4AndVertexFit
		//P4 fits automatically constrain decaying particle masses, unless they are manually disabled
	locReaction->Set_KinFitType(d_P4AndVertexFit);

	// Highly Recommended: When generating particle combinations, reject all beam photons that match to a different RF bunch
	locReaction->Set_NumPlusMinusRFBunches(1); //1: 3 bunches, -1, 0, 1

	// Highly Recommended: Cut on number of extra "good" tracks. "Good" tracks are ones that survive the "PreSelect" (or user custom) factory.
		// Important: Keep cut large: Can have many ghost and accidental tracks that look "good"
	locReaction->Set_MaxExtraGoodTracks(4);

	// Highly Recommended: Enable ROOT TTree output for this DReaction
	// string is file name (must end in ".root"!!): doen't need to be unique, feel free to change
	locReaction->Enable_TTreeOutput("tree_p2k.root", false); //true/false: do/don't save unused hypotheses

	/**************************************************** p2k Analysis Actions ****************************************************/

	// Recommended: Analysis actions automatically performed by the DAnalysisResults factories to histogram useful quantities.
		//These actions are executed sequentially, and are executed on each surviving (non-cut) particle combination 
		//Pre-defined actions can be found in ANALYSIS/DHistogramActions_*.h and ANALYSIS/DCutActions.h
		//If a histogram action is repeated, it should be created with a unique name (string) to distinguish them
	
	// For the mass histogramming/cutting
	std::deque<Particle_t> MyPhi;
        MyPhi.push_back(KPlus); MyPhi.push_back(KMinus);
        std::deque<Particle_t> MyLambda;
        MyLambda.push_back(KMinus); MyLambda.push_back(Proton);

	// HISTOGRAM PID
	locReaction->Add_AnalysisAction(new DHistogramAction_PID(locReaction));

	// CUT PID
	// SYS_TOF, SYS_BCAL, SYS_FCAL, ...: DetectorSystem_t: Defined in libraries/include/GlueX.h
	// locReaction->Add_AnalysisAction(new DCutAction_EachPIDFOM(locReaction, 5.73303E-7));
	// locReaction->Add_AnalysisAction(new DCutAction_PIDTimingBeta(locReaction, 0.0, 0.9, Neutron, SYS_BCAL)); //min/max beta cut for neutrons
	// locReaction->Add_AnalysisAction(new DCutAction_NoPIDHit(locReaction, KPlus)); //for K+ candidates, cut tracks with no PID hit
	locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 0.60, Proton, SYS_TOF));  //false: measured data
        locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 1.00, Proton, SYS_BCAL)); //false: measured data
        locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 2.00, Proton, SYS_FCAL)); //false: measured data
        locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 0.40, KPlus, SYS_TOF));   //false: measured data
        locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 0.75, KPlus, SYS_BCAL));  //false: measured data
        locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 2.50, KPlus, SYS_FCAL));  //false: measured data
        locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 0.40, KMinus, SYS_TOF));  //false: measured data
        locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 0.75, KMinus, SYS_BCAL)); //false: measured data
        locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 2.50, KMinus, SYS_FCAL)); //false: measured data

	// HISTOGRAM MASSES //false/true: measured/kinfit data
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, MyPhi, false, 1000, 0.9, 2.4, "Pre_KinFit"));

	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, 0.0)); //0% confidence level cut //require kinematic fit converges
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, MyPhi, false, 1000, 0.9, 2.4, "Post_KinFit"));

	// Kinematics
	locReaction->Add_AnalysisAction(new DHistogramAction_ParticleComboKinematics(locReaction, false)); //false: measured data
	locReaction->Add_AnalysisAction(new DHistogramAction_ParticleComboKinematics(locReaction, true, "KinFit")); //true: kinematic-fit data
	locReaction->Add_AnalysisAction(new DHistogramAction_TrackVertexComparison(locReaction));

	_data.push_back(locReaction); //Register the DReaction with the factory

	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DReaction_factory_p2k::fini(void)
{
	for(size_t loc_i = 0; loc_i < dReactionStepPool.size(); ++loc_i)
		delete dReactionStepPool[loc_i]; //cleanup memory
	return NOERROR;
}
