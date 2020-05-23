// $Id$
//
//    File: DCustomAction_dirc_reactions.h
//

#ifndef _DCustomAction_dirc_reactions_
#define _DCustomAction_dirc_reactions_

#include <string>
#include <iostream>

#include "TH1.h"
#include "TH3.h"
#include "TLorentzRotation.h"

#include "JANA/JEventLoop.h"
#include "JANA/JApplication.h"

#include "DIRC/DDIRCGeometry.h"
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
		void Run_Update(JEventLoop* locEventLoop);

	private:

		bool Perform_Action(JEventLoop* locEventLoop, const DParticleCombo* locParticleCombo);

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

		//Store any histograms as member variables here
		TH1I *hDiff, *hDiffDirect, *hDiffReflected;
		TH1I *hDeltaThetaC, *hDeltaThetaCDirect, *hDeltaThetaCReflected;
		TH1I *hNphC, *hThetaC, *hLikelihood, *hLikelihoodDiff;
		TH2I *hThetaCVsP, *hLikelihoodDiffVsP, *hReactionLikelihoodDiffVsP;
		TH2I *hDeltaThetaCVsP, *hDeltaThetaCVsPDirect, *hDeltaThetaCVsPReflected;
		TH2I *hDeltaThetaCVsChannel;
		TH2I *hDeltaThetaCVsChannelMap[48];
		TH2I *hDeltaThetaCDirectVsChannelMap[48], *hDeltaThetaCReflectedVsChannelMap[48];
		TH2I *hDeltaThetaCDirectVsPMTMap[48], *hDeltaThetaCReflectedVsPMTMap[48];
		TH2I *hDeltaThetaCVsDeltaYMap[48];

		TH2I *hExtrapolatedBarHitXY, *hExtrapolatedBarHitXY_PreCut;
		TH2I *hDeltaTOF[48][40];
		TH1I *hDiffMap[48][40], *hDiffMapDirect[48][40], *hDiffMapReflected[48][40];
		TH1I *hHitTimeMap[48][40], *hHitTimeMapAll[48][40];
		TH1I *hNphCMap[48][40]; //, *hNphCMapSlot4[48][40], *hNphCMapSlot5[48][40];
		//TH3S *hPixelHitMap3D[48][40];
		TH2S *hPixelHitMap[48][40], *hPixelHitMapReflected[48][40];
		//TH2S *hPixelHitMapAll[48][40], *hPixelHitMapAllReflected[48][40];
		//TH2I *hPixelHitTimeMap[48][40];
		TH2I *hThetaCVsPMap[48][40], *hReactionLikelihoodDiffVsPMap[48][40];
		TH2I *hDeltaThetaCVsPMap[48][40], *hDeltaThetaCVsPMapDirect[48][40], *hDeltaThetaCVsPMapReflected[48][40];
};

#endif // _DCustomAction_dirc_reactions_

