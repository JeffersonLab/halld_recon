// $Id$
//
//    File: DCustomAction_dirc_track_pair.h
//

#ifndef _DCustomAction_dirc_track_pair_
#define _DCustomAction_dirc_track_pair_

#include <string>
#include <iostream>

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

class DCustomAction_dirc_track_pair : public DAnalysisAction
{
	public:

                DCustomAction_dirc_track_pair(const DReaction* locReaction, bool locUseKinFitResultsFlag, int locParticleComboStepIndex, int locParticleIndex1, int locParticleIndex2, string locActionUniqueString = "") : 
                DAnalysisAction(locReaction, "Custom_dirc_track_pair", locUseKinFitResultsFlag, locActionUniqueString), dParticleComboStepIndex(locParticleComboStepIndex), dParticleIndex1(locParticleIndex1), dParticleIndex2(locParticleIndex2) {}

		void Initialize(JEventLoop* locEventLoop);
		void Run_Update(JEventLoop* locEventLoop);

	private:

		bool Perform_Action(JEventLoop* locEventLoop, const DParticleCombo* locParticleCombo);

                // indices for particles to histogram
                int dParticleComboStepIndex, dParticleIndex1, dParticleIndex2;

		// need PID algos for DIRC
                const DParticleID* dParticleID;
		const DDIRCGeometry* dDIRCGeometry;

		//Store any histograms as member variables here
		TH2I *hDeltaBarX_SameBox;
		TH1I *hComboCounter;
};

#endif // _DCustomAction_dirc_track_pair_

