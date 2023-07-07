// $Id$
//
//    File: DReaction_factory_PhiSkim.cc
// Created: Wed Mar 11 20:34:22 EDT 2015
// Creator: jrsteven (on Linux halldw1.jlab.org 2.6.32-504.8.1.el6.x86_64 x86_64)
//

#include "DReaction_factory_PhiSkim.h"

void DReaction_factory_PhiSkim::PIDCuts(DReaction* locReaction)
{
  locReaction->Add_AnalysisAction(new DHistogramAction_PID(locReaction, false));
  locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 0.3, Proton, SYS_TOF));
  locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 1.0, Proton, SYS_BCAL));
  locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 1.0, Proton, SYS_FCAL));
  locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 0.3, KPlus, SYS_TOF));
  locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 0.5, KPlus, SYS_BCAL));
  locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 2.0, KPlus, SYS_FCAL));
  locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 0.3, KMinus, SYS_TOF));
  locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 0.5, KMinus, SYS_BCAL));
  locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 2.0, KMinus, SYS_FCAL));
  locReaction->Add_AnalysisAction(new DHistogramAction_PID(locReaction, false, "PostPIDCuts"));

  // Cut low beam energy as tagger settings change during 2017-01
  //	locReaction->Add_AnalysisAction(new DCutAction_BeamEnergy(locReaction, false, 7.0, 12.0));
}
	


//------------------
// brun
//------------------
jerror_t DReaction_factory_PhiSkim::brun(JEventLoop* locEventLoop, int32_t locRunNumber)
{
  vector<double> locBeamPeriodVector;
  locEventLoop->GetCalib("PHOTON_BEAM/RF/beam_period", locBeamPeriodVector);
  dBeamBunchPeriod = locBeamPeriodVector[0];

  return NOERROR;
}

//------------------
// init
//------------------
jerror_t DReaction_factory_PhiSkim::evnt(JEventLoop* locEventLoop, uint64_t locEventNumber)
{
  // Make as many DReaction objects as desired
  DReactionStep* locReactionStep = NULL;
  DReaction* locReaction;

  // DOCUMENTATION:
  // ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software
  // DReaction factory: https://halldweb1.jlab.org/wiki/index.php/Analysis_DReaction

  /**************************************************** kk_skim_excl Reaction Steps ****************************************************/

  locReaction = new DReaction("kk_skim_excl"); //needs to be a unique name for each DReaction object, CANNOT (!) be "Thrown"

  // g, p -> phi, p
  locReactionStep = new DReactionStep();
  locReactionStep->Set_InitialParticleID(Gamma);
  locReactionStep->Set_TargetParticleID(Proton);
  locReactionStep->Add_FinalParticleID(phiMeson);
  locReactionStep->Add_FinalParticleID(Proton); 
  locReaction->Add_ReactionStep(locReactionStep);
  dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

  // phi -> K+ K-
  locReactionStep = new DReactionStep();
  locReactionStep->Set_InitialParticleID(phiMeson);
  locReactionStep->Add_FinalParticleID(KPlus);
  locReactionStep->Add_FinalParticleID(KMinus);
  locReactionStep->Set_KinFitConstrainInitMassFlag(false);
  locReaction->Add_ReactionStep(locReactionStep);
  dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

  /**************************************************** p3pi_preco_2FCAL Control Settings ****************************************************/

  // KINFIT
  locReaction->Set_KinFitType(d_P4AndVertexFit); //simultaneously constrain apply four-momentum conservation, invariant masses, and common-vertex constraints

  // Highly Recommended: When generating particle combinations, reject all beam photons that match to a different RF bunch
  locReaction->Set_NumPlusMinusRFBunches(0); // 0: only in time

  /**************************************************** p3pi_preco_2FCAL Analysis Actions ****************************************************/

  // PID
  PIDCuts(locReaction);

  // MASSES
  locReaction->Add_AnalysisAction(new DHistogramAction_MissingMassSquared(locReaction, false, 1000, -0.1, 0.1));
  locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, phiMeson, false, 300, 0.9, 1.5, "Phi"));
  locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, phiMeson, true, 300, 0.9, 1.5, "Phi_KinFit"));

  // Kinematic Fit Results
  locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05, true)); //5% confidence level cut on pull histograms only
  //  locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, 5.73303E-7)); // confidence level cut //+/- 5 sigma
  // locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, 0.05));
  locReaction->Add_AnalysisAction(new DCutAction_KinFitChiSq(locReaction, 5.0));  
  locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05 ,true)); //5% confidence level cut on pull histograms only.  This means put a cut on the pull to draw the histos, don't actually cut the events.
  
  // MASSES, POST-KINFIT
  locReaction->Add_AnalysisAction(new DHistogramAction_MissingMassSquared(locReaction, false, 1000, -0.1, 0.1, "PostKinFitCut"));
  locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, phiMeson, false, 300, 0.9, 1.5, "Phi_PostKinFitCut"));
  locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, phiMeson, true, 300, 0.9, 1.5, "Phi_KinFit_PostKinFitCut"));




  // cut around phiMeson mass
  locReaction->Add_AnalysisAction(new DCutAction_InvariantMass(locReaction,phiMeson,true,1.01,1.03));
  locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, phiMeson, true, 300, 0.9, 1.5, "Phi_KinFit_PostIMCut"));

  // Kinematics of final selection
  locReaction->Add_AnalysisAction(new DHistogramAction_ParticleComboKinematics(locReaction, false, "Final")); //false: fill histograms with measured particle data  
   string locTreeFileName = "phi_excl_skim.root";
   locReaction->Enable_TTreeOutput(locTreeFileName, true); //true/false: do/don't save unused hypotheses
  
  _data.push_back(locReaction); //Register the DReaction with the factory

  
  
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DReaction_factory_PhiSkim::fini(void)
{
  for(size_t loc_i = 0; loc_i < dReactionStepPool.size(); ++loc_i)
    delete dReactionStepPool[loc_i]; //cleanup memory
  return NOERROR;
}
