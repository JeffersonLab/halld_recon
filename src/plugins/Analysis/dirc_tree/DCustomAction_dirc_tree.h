// $Id$
//
//    File: DCustomAction_dirc_tree.h
//

#ifndef _DCustomAction_dirc_tree_
#define _DCustomAction_dirc_tree_

#include <string>
#include <iostream>

#include "TH1.h"
#include "TLorentzRotation.h"

#include <JANA/JEvent.h>
#include "JANA/JApplication.h"

#include "DIRC/DDIRCGeometry.h"
#include "DIRC/DDIRCLut.h"
#include "ANALYSIS/DAnalysisAction.h"
#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DParticleCombo.h"
#include "ANALYSIS/DAnalysisUtilities.h"

#include "DrcHit.h"
#include "DrcEvent.h"

#include <TVector3.h>
#include <TROOT.h>
#include <TClonesArray.h>

using namespace std;

class DCustomAction_dirc_tree : public DAnalysisAction
{
public:

  DCustomAction_dirc_tree(const DReaction* locReaction, bool locUseKinFitResultsFlag, Particle_t locPID, int locParticleComboStepIndex, string locActionUniqueString = "") :
    DAnalysisAction(locReaction, "Custom_dirc_reactions", locUseKinFitResultsFlag, locActionUniqueString), dParticleComboStepIndex(locParticleComboStepIndex), dPID(locPID) {}

  void Initialize(const std::shared_ptr<const JEvent>& locEvent);

private:

  bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo);

  // Optional: Useful utility functions.
  const DAnalysisUtilities* dAnalysisUtilities;

  // need PID algos for DIRC
  const DParticleID* dParticleID;
  const DDIRCGeometry* dDIRCGeometry;
  // DDIRCLut for likelihood calculation
  const DDIRCLut* dDIRCLut;
  deque<Particle_t> dFinalStatePIDs;

  // indices for particles to histogram
  int dParticleComboStepIndex, dParticleIndex;
  Particle_t dPID;

  bool DIRC_TRUTH_BARHIT, DIRC_FILL_BAR_MAP;
  TClonesArray *fcEvent;
  DrcEvent *fEvent;
  TTree *fTree;
};

#endif // _DCustomAction_dirc_tree_

