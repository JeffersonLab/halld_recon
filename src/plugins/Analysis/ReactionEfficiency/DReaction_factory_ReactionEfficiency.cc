// $Id$
//
//    File: DReaction_factory_ReactionEfficiency.cc
// Created: Wed Jun 19 16:52:57 EDT 2019
// Creator: jrsteven (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "DReaction_factory_ReactionEfficiency.h"

//------------------
// evnt
//------------------
jerror_t DReaction_factory_ReactionEfficiency::evnt(JEventLoop* locEventLoop, uint64_t locEventNumber)
{
	// Make as many DReaction objects as desired
	DReactionStep* locReactionStep = NULL;
	DReaction* locReaction = NULL; //create with a unique name for each DReaction object. CANNOT (!) be "Thrown"

	double locMinKinFitFOM = 1e-4;
	gPARMS->SetDefaultParameter("REACTIONEFFIC:MINKINFITFOM", locMinKinFitFOM);

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software
	// DReaction factory: https://halldweb1.jlab.org/wiki/index.php/Analysis_DReaction

	/************************************************** ReactionEfficiency Reaction Definition *************************************************/

	locReaction = new DReaction("pi0pipmisspim__B1_T1_U1_M7_Effic");

	//Required: DReactionSteps to specify the channel and decay chain you want to study
		//Particles are of type Particle_t, an enum defined in sim-recon/src/libraries/include/particleType.h

	//Example: g, p -> pi+, pi-, pi0, (p)
	//Inputs: Beam, target, non-missing final-state particles (vector), missing final state particle (none by default), bool inclusive_flag = false by default
	locReactionStep = new DReactionStep(Gamma, Proton, {Pi0, PiPlus, Proton}, PiMinus);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	//Example: pi0 -> g, g
	//Inputs: Decaying, non-missing final-state particles (vector), missing final state particle (none by default), bool inclusive_flag = false by default
	locReactionStep = new DReactionStep(Pi0, {Gamma, Gamma});
	locReactionStep->Set_KinFitConstrainInitMassFlag(false); //default: true //ignored if p4 not fit or is beam //phi, omega not constrained regardless
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	/**************************************************** ReactionEfficiency Control Settings ****************************************************/

	// Recommended: Type of kinematic fit to perform (default is d_NoFit)
		//fit types are of type DKinFitType, an enum defined in sim-recon/src/libraries/ANALYSIS/DReaction.h
		//Options: d_NoFit (default), d_P4Fit, d_VertexFit, d_P4AndVertexFit
		//P4 fits automatically constrain decaying particle masses, unless they are manually disabled
	locReaction->Set_KinFitType(d_P4AndVertexFit);

	// Highly Recommended: When generating particle combinations, reject all beam photons that match to a different RF bunch
	locReaction->Set_NumPlusMinusRFBunches(1); //1: 3 bunches, -1, 0, 1

	// Highly Recommended: Cut on number of extra "good" tracks. "Good" tracks are ones that survive the "PreSelect" (or user custom) factory.
		// Important: Keep cut large: Can have many ghost and accidental tracks that look "good"
	locReaction->Set_MaxExtraGoodTracks(1);

	// Highly Recommended: Enable ROOT TTree output for this DReaction
	// string is file name (must end in ".root"!!): doen't need to be unique, feel free to change
	locReaction->Enable_TTreeOutput("tree_pi0pipmisspim__B1_T1_U1_M7_Effic.root", true); //true/false: do/don't save unused hypotheses

	/**************************************************** ReactionEfficiency Analysis Actions ****************************************************/

	// Recommended: Analysis actions automatically performed by the DAnalysisResults factories to histogram useful quantities.
		//These actions are executed sequentially, and are executed on each surviving (non-cut) particle combination 
		//Pre-defined actions can be found in ANALYSIS/DHistogramActions_*.h and ANALYSIS/DCutActions.h
		//If a histogram action is repeated, it should be created with a unique name (string) to distinguish them	

	// HISTOGRAM MASSES //false/true: measured/kinfit data
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, Pi0, false, 600, 0.0, 0.3, "Pi0_PreKinFit"));

	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges

	// CUSTOM ACTION TO REDUCE OUTPUT SIZE
	deque<int> locRecoilIndices;  locRecoilIndices.push_back(2);
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, false, locRecoilIndices, 0.2, 1.2, "OmegaRecoil"));
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, true, locRecoilIndices, 0.2, 1.2, "OmegaRecoil_KinFit"));

	// HISTOGRAM MASSES //false/true: measured/kinfit data
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, Pi0, false, 600, 0.0, 0.3, "Pi0_PostKinFit"));

	_data.push_back(locReaction); //Register the DReaction with the factory


	/**************************************************** pi0pimmisspip__B1_T1_U1_M7_Effic ****************************************************/

	locReaction = new DReaction("pi0pimmisspip__B1_T1_U1_M7_Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {Pi0, PiMinus, Proton}, PiPlus);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak
	locReactionStep = new DReactionStep(Pi0, {Gamma, Gamma});
	locReactionStep->Set_KinFitConstrainInitMassFlag(false); 
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_pi0pimmisspip__B1_T1_U1_M7_Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses

	// HISTOGRAM MASSES //false/true: measured/kinfit data
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, Pi0, false, 600, 0.0, 0.3, "Pi0_PreKinFit"));

	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges

	// CUSTOM ACTION TO REDUCE OUTPUT SIZE
	locRecoilIndices.clear();  locRecoilIndices.push_back(2);
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, false, locRecoilIndices, 0.2, 1.2, "OmegaRecoil"));
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, true, locRecoilIndices, 0.2, 1.2, "OmegaRecoil_KinFit"));

	// HISTOGRAM MASSES //false/true: measured/kinfit data
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, Pi0, false, 600, 0.0, 0.3, "Pi0_PostKinFit"));

	_data.push_back(locReaction); //Register the DReaction with the factory

	/**************************************************** kpkmmissprot__B1_T1_U1_Effic ****************************************************/
	
	locReaction = new DReaction("kpkmmissprot__B1_T1_U1_Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {KPlus, KMinus}, Proton);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak
	
	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_kpkmmissprot__B1_T1_U1_Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges
	
	// CUSTOM ACTION TO REDUCE OUTPUT SIZE
        locRecoilIndices.clear();  locRecoilIndices.push_back(0); // KPlus
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, false, locRecoilIndices, 1.0, 2.0, "Lambda1520Recoil"));
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, true, locRecoilIndices, 1.0, 2.0, "Lambda1520Recoil_KinFit"));
	
	//_data.push_back(locReaction); //Register the DReaction with the factory

	/**************************************************** kpmisskm__B1_T1_U1_Lambda1520Effic ****************************************************/
	
	locReaction = new DReaction("kpmisskm__B1_T1_U1_Lambda1520Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {KPlus, Proton}, KMinus);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak
	
	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_kpmisskm__B1_T1_U1_Lambda1520Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges
	
	// CUSTOM ACTION TO REDUCE OUTPUT SIZE
        locRecoilIndices.clear();  locRecoilIndices.push_back(0); // KPlus
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, false, locRecoilIndices, 1.0, 2.0, "Lambda1520Recoil"));
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, true, locRecoilIndices, 1.0, 2.0, "Lambda1520Recoil_KinFit"));
	
	_data.push_back(locReaction); //Register the DReaction with the factory

	/**************************************************** kpmisskm__B1_T1_U1_PhiEffic ****************************************************/
	
	locReaction = new DReaction("kpmisskm__B1_T1_U1_PhiEffic");
	locReactionStep = new DReactionStep(Gamma, Proton, {KPlus, Proton}, KMinus);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak
	
	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_kpmisskm__B1_T1_U1_PhiEffic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges
	
	// CUSTOM ACTION TO REDUCE OUTPUT SIZE
        locRecoilIndices.clear();  locRecoilIndices.push_back(1); // Proton
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, false, locRecoilIndices, 0.8, 1.3, "PhiRecoil"));
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, true, locRecoilIndices, 0.8, 1.3, "PhiRecoil_KinFit"));
	
	_data.push_back(locReaction); //Register the DReaction with the factory

	/**************************************************** kmmisskp__B1_T1_U1_PhiEffic ****************************************************/
	
	locReaction = new DReaction("kmmisskp__B1_T1_U1_PhiEffic");
	locReactionStep = new DReactionStep(Gamma, Proton, {KMinus, Proton}, KPlus);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak
	
	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_kmmisskp__B1_T1_U1_PhiEffic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges
	
	// CUSTOM ACTION TO REDUCE OUTPUT SIZE
        locRecoilIndices.clear();  locRecoilIndices.push_back(1); // Proton
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, false, locRecoilIndices, 0.8, 1.3, "PhiRecoil"));
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, true, locRecoilIndices, 0.8, 1.3, "PhiRecoil_KinFit"));
	
	_data.push_back(locReaction); //Register the DReaction with the factory

	/**************************************************** gpimkpmissprot__B1_T1_U1_Effic ****************************************************/
	
	locReaction = new DReaction("gpimkpmissprot__B1_T1_U1_Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {Gamma, PiMinus, KPlus}, Proton);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak
	
	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_gpimkpmissprot__B1_T1_U1_Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges
	
	// CUSTOM ACTION TO REDUCE OUTPUT SIZE
        locRecoilIndices.clear();  locRecoilIndices.push_back(2); // KPlus
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, false, locRecoilIndices, 0.5, 1.5, "Sigma0Recoil"));
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, true, locRecoilIndices, 0.5, 1.5, "Sigma0Recoil_KinFit"));
	
	_data.push_back(locReaction); //Register the DReaction with the factory

	/**************************************************** kpsigmamissprot__B1_T1_U1_Effic ****************************************************/
	
	locReaction = new DReaction("kpsigmamissprot__B1_T1_U1_Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {Sigma0, KPlus});
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak
	
	locReactionStep = new DReactionStep(Sigma0, {Lambda, Gamma});
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(Lambda, {PiMinus}, Proton);
	locReactionStep->Set_KinFitConstrainInitMassFlag(false);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_kpsigmamissprot__B1_T1_U1_Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges
	
	// CUSTOM ACTION TO REDUCE OUTPUT SIZE
        locRecoilIndices.clear();  locRecoilIndices.push_back(1); // KPlus
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, false, locRecoilIndices, 0.5, 1.5, "Sigma0Recoil"));
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, true, locRecoilIndices, 0.5, 1.5, "Sigma0Recoil_KinFit"));
	
	_data.push_back(locReaction); //Register the DReaction with the factory


	/**************************************************** antilamblambmissprot__B1_T1_U1_Effic ****************************************************/
	
	locReaction = new DReaction("antilamblambmissprot__B1_T1_U1_Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {AntiLambda, Lambda, Proton});
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(AntiLambda, {PiPlus, AntiProton});
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(Lambda, {PiMinus}, Proton);
	locReactionStep->Set_KinFitConstrainInitMassFlag(false);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_antilamblambmissprot__B1_T1_U1_Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges

	// CUT ACTION FOR LAMBDA
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, AntiLambda, false, 100, 0.8, 1.3, "AntiLambdaMass"));
	locReaction->Add_AnalysisAction(new DCutAction_InvariantMass(locReaction, AntiLambda, false, 0.95, 1.25, "AntiLambdaMassCut"));
	
	_data.push_back(locReaction); //Register the DReaction with the factory

	
	/**************************************************** antilamblambmisspim__B1_T1_U1_Effic ****************************************************/
	
	locReaction = new DReaction("antilamblambmisspim__B1_T1_U1_Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {AntiLambda, Lambda, Proton});
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(AntiLambda, {PiPlus, AntiProton});
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(Lambda, {Proton}, PiMinus);
	locReactionStep->Set_KinFitConstrainInitMassFlag(false);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_antilamblambmisspim__B1_T1_U1_Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges
	
	// CUT ACTION FOR LAMBDA
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, AntiLambda, false, 100, 0.8, 1.3, "AntiLambdaMass"));
	locReaction->Add_AnalysisAction(new DCutAction_InvariantMass(locReaction, AntiLambda, false, 0.95, 1.25, "AntiLambdaMassCut"));

	_data.push_back(locReaction); //Register the DReaction with the factory

	
	/**************************************************** antilamblambmissprot__B1_T1_U1_Effic ****************************************************/
	
	locReaction = new DReaction("antilamblambmissantip__B1_T1_U1_Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {AntiLambda, Lambda, Proton});
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(AntiLambda, {PiPlus}, AntiProton);
	locReactionStep->Set_KinFitConstrainInitMassFlag(false);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(Lambda, {PiMinus, Proton});
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_antilamblambmissantip__B1_T1_U1_Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges

	// CUT ACTION FOR LAMBDA
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, Lambda, false, 100, 0.8, 1.3, "LambdaMass"));
	locReaction->Add_AnalysisAction(new DCutAction_InvariantMass(locReaction, Lambda, false, 0.95, 1.25, "LambdaMassCut"));
	
	_data.push_back(locReaction); //Register the DReaction with the factory
	

	/**************************************************** antilamblambmisspip__B1_T1_U1_Effic ****************************************************/
	
	locReaction = new DReaction("antilamblambmisspip__B1_T1_U1_Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {AntiLambda, Lambda, Proton});
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(AntiLambda, {AntiProton}, PiPlus);
	locReactionStep->Set_KinFitConstrainInitMassFlag(false);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(Lambda, {PiMinus, Proton});
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_antilamblambmisspip__B1_T1_U1_Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges

	// CUT ACTION FOR LAMBDA
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, Lambda, false, 100, 0.8, 1.3, "LambdaMass"));
	locReaction->Add_AnalysisAction(new DCutAction_InvariantMass(locReaction, Lambda, false, 0.95, 1.25, "LambdaMassCut"));
	
	_data.push_back(locReaction); //Register the DReaction with the factory


	/**************************************************** pi0lambkpmiss__B1_T1_U1_Effic ****************************************************/
	
	locReaction = new DReaction("pi0lambkpmiss__B1_T1_U1_Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {Pi0, PiMinus, Proton}, KPlus);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(Pi0, {Gamma, Gamma});
        locReaction->Add_ReactionStep(locReactionStep);
        dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak	

	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_pi0lambkpmiss__B1_T1_U1_Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges

	// CUSTOM ACTION TO REDUCE OUTPUT SIZE
        locRecoilIndices.clear();  locRecoilIndices.push_back(1); locRecoilIndices.push_back(2); // Lambda = PiMinus+Proton 
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, false, locRecoilIndices, 0.5, 1.3, "K*Recoil"));
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, true, locRecoilIndices, 0.75, 1.15, "K*Recoil_KinFit"));

	// CUT ACTION FOR LAMBDA
	deque<Particle_t> locLambdaIndices;  locLambdaIndices.push_back(Proton); locLambdaIndices.push_back(PiMinus);
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, 0, locLambdaIndices, false, 100, 0.8, 1.3, "LambdaMass"));
	locReaction->Add_AnalysisAction(new DCutAction_InvariantMass(locReaction, 0, locLambdaIndices, false, 0.95, 1.25, "LambdaMassCut"));

	_data.push_back(locReaction); //Register the DReaction with the factory

	
	/**************************************************** pi0lambpmiss__B1_T1_U1_Effic ****************************************************/
	
	locReaction = new DReaction("pi0lambpmiss__B1_T1_U1_Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {Pi0, PiMinus, KPlus}, Proton);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(Pi0, {Gamma, Gamma});
        locReaction->Add_ReactionStep(locReactionStep);
        dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak	

	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_pi0lambpmiss__B1_T1_U1_Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges
	
	// CUSTOM ACTION TO REDUCE OUTPUT SIZE
        locRecoilIndices.clear();  locRecoilIndices.push_back(0); locRecoilIndices.push_back(2); // K* = Pi0+KPlus 
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, true, locRecoilIndices, 0.9, 1.3, "LambdaRecoil_KinFit"));
	
	_data.push_back(locReaction); //Register the DReaction with the factory


	/**************************************************** pi0lambpimmiss__B1_T1_U1_Effic ****************************************************/
	
	locReaction = new DReaction("pi0lambpimmiss__B1_T1_U1_Effic");
	locReactionStep = new DReactionStep(Gamma, Proton, {Pi0, KPlus, Proton}, PiMinus);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	locReactionStep = new DReactionStep(Pi0, {Gamma, Gamma});
        locReaction->Add_ReactionStep(locReactionStep);
        dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak	

	locReaction->Set_KinFitType(d_P4AndVertexFit);
	locReaction->Set_NumPlusMinusRFBunches(1); // B1
	locReaction->Set_MaxExtraGoodTracks(1); // T1
	locReaction->Enable_TTreeOutput("tree_pi0lambpimmiss__B1_T1_U1_Effic.root", true); // U1 = true -> true/false: do/don't save unused hypotheses
	
	// KINEMATIC FIT
	locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, locMinKinFitFOM)); //0% confidence level cut //require kinematic fit converges
	
	// CUSTOM ACTION TO REDUCE OUTPUT SIZE
        locRecoilIndices.clear();  locRecoilIndices.push_back(0); locRecoilIndices.push_back(2); // K* = Pi0+KPlus 
	locReaction->Add_AnalysisAction(new DCustomAction_RecoilMass(locReaction, true, locRecoilIndices, 0.9, 1.3, "LambdaRecoil_KinFit"));
	
	_data.push_back(locReaction); //Register the DReaction with the factory

	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DReaction_factory_ReactionEfficiency::fini(void)
{
	for(size_t loc_i = 0; loc_i < dReactionStepPool.size(); ++loc_i)
		delete dReactionStepPool[loc_i]; //cleanup memory
	return NOERROR;
}

