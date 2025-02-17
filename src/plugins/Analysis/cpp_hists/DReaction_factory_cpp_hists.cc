// $Id$
//
//    File: DReaction_factory_cpp_hists.cc
// Created: Tue May  3 09:24:43 EDT 2022
// Creator: aaustreg (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//


#include "DReaction_factory_cpp_hists.h"

//------------------
// Process
//------------------
void DReaction_factory_cpp_hists::Process(const std::shared_ptr<const JEvent>& event)
{
	// Make as many DReaction objects as desired
	DReactionStep* locReactionStep = NULL;
	DReaction* locReaction = NULL; //create with a unique name for each DReaction object. CANNOT (!) be "Thrown"

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software
	// DReaction factory: https://halldweb1.jlab.org/wiki/index.php/Analysis_DReaction

	/************************************************** cpp_hists Reaction Definition *************************************************/

	locReaction = new DReaction("cpp_hists");

	//Required: DReactionSteps to specify the channel and decay chain you want to study
	//Particles are of type Particle_t, an enum defined in sim-recon/src/libraries/include/particleType.h

	//Example: g, p -> pi+, pi-, pi0, (p)
	//Inputs: Beam, target, non-missing final-state particles (vector), missing final state particle (none by default), bool inclusive_flag = false by default
	locReactionStep = new DReactionStep(Gamma, Pb208, {PiPlus, PiMinus}, Pb208);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak


	/**************************************************** cpp_hists Control Settings ****************************************************/

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
	locReaction->Set_NumPlusMinusRFBunches(0); //1: 3 bunches, -1, 0, 1

	// Highly Recommended: Cut on number of extra "good" tracks. "Good" tracks are ones that survive the "PreSelect" (or user custom) factory.
		// Important: Keep cut large: Can have many ghost and accidental tracks that look "good"
	locReaction->Set_MaxExtraGoodTracks(0);

	// Highly Recommended: Enable ROOT TTree output for this DReaction
	// string is file name (must end in ".root"!!): doen't need to be unique, feel free to change
	// locReaction->Enable_TTreeOutput("tree_cpp_hists.root", false); //true/false: do/don't save unused hypotheses

	/**************************************************** cpp_hists Analysis Actions ****************************************************/

	
	// Recommended: Analysis actions automatically performed by the DAnalysisResults factories to histogram useful quantities.
		//These actions are executed sequentially, and are executed on each surviving (non-cut) particle combination 
		//Pre-defined actions can be found in ANALYSIS/DHistogramActions_*.h and ANALYSIS/DCutActions.h
		//If a histogram action is repeated, it should be created with a unique name (string) to distinguish them

	// HISTOGRAM PID
	locReaction->Add_AnalysisAction(new DHistogramAction_PID(locReaction));

	// CUT PID
	// SYS_TOF, SYS_BCAL, SYS_FCAL, ...: DetectorSystem_t: Defined in libraries/include/GlueX.h
	// locReaction->Add_AnalysisAction(new DCutAction_EachPIDFOM(locReaction, 5.73303E-7));
	// locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 1.0, Proton, SYS_TOF)); //cut at delta-t +/- 1.0 //false: measured data
	// locReaction->Add_AnalysisAction(new DCutAction_PIDTimingBeta(locReaction, 0.0, 0.9, Neutron, SYS_BCAL)); //min/max beta cut for neutrons
	// locReaction->Add_AnalysisAction(new DCutAction_NoPIDHit(locReaction, KPlus)); //for K+ candidates, cut tracks with no PID hit

	// Kinematics
	locReaction->Add_AnalysisAction(new DHistogramAction_ParticleComboKinematics(locReaction, false)); //false: measured data
	// locReaction->Add_AnalysisAction(new DHistogramAction_ParticleComboKinematics(locReaction, true, "KinFit")); //true: kinematic-fit data
	locReaction->Add_AnalysisAction(new DHistogramAction_TrackVertexComparison(locReaction));
	
	// HISTOGRAM MASSES //false/true: measured/kinfit data
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, {PiPlus, PiMinus}, false, 1000, 0.0, 2.0, "2Pi_PreKinFit"));
	locReaction->Add_AnalysisAction(new DHistogramAction_MissingMass(locReaction, false, 1000, 190, 200, "PreKinFit"));

	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, 0.01)); //1% confidence level cut //require kinematic fit converges

	// HISTOGRAM MASSES //false/true: measured/kinfit data
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, {PiPlus, PiMinus}, false, 1000, 0.0, 2.0, "2Pi_PostKinFit"));
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, {PiPlus, PiMinus}, true, 1000, 0.0, 2.0, "2Pi_KinFit"));
	locReaction->Add_AnalysisAction(new DHistogramAction_MissingMass(locReaction, false, 1000, 190, 200, "PostKinFit"));
	
	// Custom histograms
	locReaction->Add_AnalysisAction(new DCustomAction_cpp_hists(locReaction, true));

	Insert(locReaction); //Register the DReaction with the factory

	return;
}

//------------------
// Finish
//------------------
void DReaction_factory_cpp_hists::Finish()
{
	for(size_t loc_i = 0; loc_i < dReactionStepPool.size(); ++loc_i)
		delete dReactionStepPool[loc_i]; //cleanup memory
	return;
}

