// $Id$
//
//    File: DCustomAction_dirc_reactions.h
//

#ifndef _DCustomAction_dirc_reactions_
#define _DCustomAction_dirc_reactions_

#include <string>
#include <iostream>

#include "TH1.h"
#include "TLorentzRotation.h"

#include "JANA/JEventLoop.h"
#include "JANA/JApplication.h"

#include "DIRC/DDIRCLut.h"
#include "ANALYSIS/DAnalysisAction.h"
#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DParticleCombo.h"
#include "ANALYSIS/DAnalysisUtilities.h"

using namespace std;
using namespace jana;

class DCustomAction_dirc_reactions : public DAnalysisAction
{
	public:

                DCustomAction_dirc_reactions(const DReaction* locReaction, bool locUseKinFitResultsFlag, Particle_t locPID, int locParticleComboStepIndex, int locParticleIndex, string locActionUniqueString = "") : 
	        DAnalysisAction(locReaction, "Custom_dirc_reactions", locUseKinFitResultsFlag, locActionUniqueString), dParticleComboStepIndex(locParticleComboStepIndex), dParticleIndex(locParticleIndex), dPID(locPID) {}

		void Initialize(JEventLoop* locEventLoop);

	private:

		bool Perform_Action(JEventLoop* locEventLoop, const DParticleCombo* locParticleCombo);

		// Optional: Useful utility functions.
		const DAnalysisUtilities* dAnalysisUtilities;

		// need PID algos for DIRC
                const DParticleID* dParticleID;
		
		// DDIRCLut for likelihood calculation
		const DDIRCLut* dDIRCLut;
		deque<Particle_t> dFinalStatePIDs;

		// indices for particles to histogram
		int dParticleComboStepIndex, dParticleIndex;
		Particle_t dPID;

		bool DIRC_TRUTH_BARHIT;

		//Store any histograms as member variables here
		TH1I *hDiff, *hNphC, *hThetaC, *hDeltaThetaC, *hLikelihood, *hLikelihoodDiff;
		TH2I *hThetaCVsP, *hDeltaThetaCVsP, *hLikelihoodDiffVsP, *hReactionLikelihoodDiffVsP;
};

#endif // _DCustomAction_dirc_reactions_

