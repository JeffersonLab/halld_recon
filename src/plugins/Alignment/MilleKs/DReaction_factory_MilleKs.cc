// $Id$
//
//    File: DReaction_factory_MilleKs.cc
// Created: Mon Feb 22 18:20:38 EST 2021
// Creator: keigo (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#include "DReaction_factory_MilleKs.h"

void DReaction_factory_MilleKs::Process(const std::shared_ptr<const JEvent> &locEvent) {

  /** MilleKs Reaction Definition **/
  DReactionStep* locReactionStep = NULL;

  /** KsKpPim **/
  DReaction* locReaction = new DReaction("KsKpPim");

  // g, p -> Ks K+ pi- p
  locReactionStep = new DReactionStep();
  locReactionStep->Set_InitialParticleID(Gamma);
  locReactionStep->Set_TargetParticleID(Proton);
  locReactionStep->Add_FinalParticleID(KShort);
  locReactionStep->Add_FinalParticleID(KPlus);
  locReactionStep->Add_FinalParticleID(PiMinus);
  locReactionStep->Add_FinalParticleID(Proton);
  locReaction->Add_ReactionStep(locReactionStep);
  dReactionStepPool.push_back(locReactionStep);  // register so will be deleted later: prevent memory leak

  // Ks -> pi+, pi-
  locReactionStep = new DReactionStep();
  locReactionStep->Set_InitialParticleID(KShort);
  locReactionStep->Add_FinalParticleID(PiPlus);
  locReactionStep->Add_FinalParticleID(PiMinus);
  locReaction->Add_ReactionStep(locReactionStep);
  dReactionStepPool.push_back(locReactionStep);  // register so will be deleted later: prevent memory leak

  /** MilleKs Control Settings **/

  // Highly Recommended: Set EventStore skim query (use with "eventstore"
  // source) This will skip creating particle combos for events that aren't in
  // the skims you list Query should be comma-separated list of skims to
  // boolean-AND together
  // locReaction->Set_EventStoreSkims("myskim1,myskim2,myskim3"); //boolean-AND
  // of skims

  // Recommended: Type of kinematic fit to perform (default is d_NoFit)
  // fit types are of type DKinFitType, an enum defined in
  // sim-recon/src/libraries/ANALYSIS/DReaction.h Options: d_NoFit (default),
  // d_P4Fit, d_VertexFit, d_P4AndVertexFit P4 fits automatically constrain
  // decaying particle masses, unless they are manually disabled

  // d_P4AndVertexFit causes segfault..
  // locReaction->Set_KinFitType(d_P4AndVertexFit);
  locReaction->Set_KinFitType(d_P4AndVertexFit);

  // Highly Recommended: When generating particle combinations, reject all beam
  // photons that match to a different RF bunch
  locReaction->Set_NumPlusMinusRFBunches(0);  // 1: 3 bunches, -1, 0, 1

  // Highly Recommended: Cut on number of extra "good" tracks. "Good" tracks are
  // ones that survive the "PreSelect" (or user custom) factory. Important: Keep
  // cut large: Can have many ghost and accidental tracks that look "good"
  // locReaction->Set_MaxExtraGoodTracks(4);

  // Highly Recommended: Enable ROOT TTree output for this DReaction
  // string is file name (must end in ".root"!!): doen't need to be unique, feel
  // free to change locReaction->Enable_TTreeOutput("tree_MilleKs.root",
  // false); //true/false: do/don't save unused hypotheses

  /** MilleKs Analysis Actions **/

  /*
  // Recommended: Analysis actions automatically performed by the
  DAnalysisResults factories to histogram useful quantities.
          //These actions are executed sequentially, and are executed on each
  surviving (non-cut) particle combination
          //Pre-defined actions can be found in ANALYSIS/DHistogramActions_*.h
  and ANALYSIS/DCutActions.h
          //If a histogram action is repeated, it should be created with a
  unique name (string) to distinguish them

  // HISTOGRAM PID
  locReaction->Add_AnalysisAction(new DHistogramAction_PID(locReaction));

  // CUT PID
  // SYS_TOF, SYS_BCAL, SYS_FCAL, ...: DetectorSystem_t: Defined in
  libraries/include/GlueX.h
  // locReaction->Add_AnalysisAction(new
  DCutAction_EachPIDFOM(locReaction, 5.73303E-7));
  // locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction,
  false, 1.0, Proton, SYS_TOF)); //cut at delta-t +/- 1.0 //false: measured data
  // locReaction->Add_AnalysisAction(new DCutAction_PIDTimingBeta(locReaction,
  0.0, 0.9, Neutron, SYS_BCAL)); //min/max beta cut for neutrons
  // locReaction->Add_AnalysisAction(new DCutAction_NoPIDHit(locReaction,
  KPlus)); //for K+ candidates, cut tracks with no PID hit

  // HISTOGRAM MASSES //false/true: measured/kinfit data
  locReaction->Add_AnalysisAction(new
  DHistogramAction_InvariantMass(locReaction, Pi0, false, 600, 0.0, 0.3,
  "Pi0_PreKinFit")); locReaction->Add_AnalysisAction(new
  DHistogramAction_MissingMass(locReaction, false, 1000, 0.7, 1.2,
  "PreKinFit"));

  // KINEMATIC FIT
  // locReaction->Add_AnalysisAction(new
  DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut
  on pull histograms only
  // locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction,
  0.0)); //0% confidence level cut //require kinematic fit converges

  // HISTOGRAM MASSES //false/true: measured/kinfit data
  //locReaction->Add_AnalysisAction(new
  DHistogramAction_InvariantMass(locReaction, Pi0, false, 600, 0.0, 0.3,
  "Pi0_PostKinFit"));
  //locReaction->Add_AnalysisAction(new
  DHistogramAction_MissingMass(locReaction, false, 1000, 0.7, 1.2,
  "PostKinFit"));

  // Kinematics
  locReaction->Add_AnalysisAction(new
  DHistogramAction_ParticleComboKinematics(locReaction, false)); //false:
  measured data
  // locReaction->Add_AnalysisAction(new
  DHistogramAction_ParticleComboKinematics(locReaction, true, "KinFit"));
  //true: kinematic-fit data locReaction->Add_AnalysisAction(new
  DHistogramAction_TrackVertexComparison(locReaction));
  */

  Insert(locReaction);  // Register the DReaction with the factory


  /** KsKmPip **/
  locReaction = new DReaction("KsKmPip");

  // g, p -> Ks K- pi+ p
  locReactionStep = new DReactionStep();
  locReactionStep->Set_InitialParticleID(Gamma);
  locReactionStep->Set_TargetParticleID(Proton);
  locReactionStep->Add_FinalParticleID(KShort);
  locReactionStep->Add_FinalParticleID(KMinus);
  locReactionStep->Add_FinalParticleID(PiPlus);
  locReactionStep->Add_FinalParticleID(Proton);
  locReaction->Add_ReactionStep(locReactionStep);
  dReactionStepPool.push_back(locReactionStep);  // register so will be deleted later: prevent memory leak

  // Ks -> pi+, pi-
  locReaction->Add_ReactionStep(dReactionStepPool[1]);

  locReaction->Set_KinFitType(d_P4AndVertexFit);
  locReaction->Set_NumPlusMinusRFBunches(0);  // 1: 3 bunches, -1, 0, 1

  Insert(locReaction);  // Register the DReaction with the factory
}

void DReaction_factory_MilleKs::Finish() {
  for (size_t loc_i = 0; loc_i < dReactionStepPool.size(); ++loc_i)
    delete dReactionStepPool[loc_i];  // cleanup memory
}
