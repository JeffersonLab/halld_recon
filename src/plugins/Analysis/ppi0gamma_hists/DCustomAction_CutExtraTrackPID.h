// $Id$
//
//    File: DCustomAction_CutExtraTrackPID.h
// Created: Thu Oct  1 21:41:32 EDT 2015
// Creator: pmatt (on Darwin Pauls-MacBook-Pro-2.local 13.4.0 i386)
//

#ifndef _DCustomAction_CutExtraTrackPID_
#define _DCustomAction_CutExtraTrackPID_

#include <string>
#include <iostream>

#include <JANA/JEvent.h>
#include "JANA/JApplication.h"

#include "ANALYSIS/DAnalysisAction.h"
#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DParticleCombo.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include "ANALYSIS/DCutActions.h"

using namespace std;

class DCustomAction_CutExtraTrackPID : public DAnalysisAction
{
	public:

		DCustomAction_CutExtraTrackPID(const DReaction* locReaction, Particle_t locExtraTrackTargetPID, string locActionUniqueString = "") : 
		DAnalysisAction(locReaction, "Custom_CutExtraTrackPID", false, locActionUniqueString), 
		dExtraTrackTargetPID(locExtraTrackTargetPID) {}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent) {
			locEvent->GetSingle(dAnalysisUtilities);
			ddEdxCutAction->Run_Update(locEvent);
		}

	private:

		Particle_t dExtraTrackTargetPID;
		map<DetectorSystem_t, double> dPIDCuts;
		DCutAction_dEdx* ddEdxCutAction;

		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo);

		// Optional: Useful utility functions.
		const DAnalysisUtilities* dAnalysisUtilities;

		//Store any histograms as member variables here
};

#endif // _DCustomAction_CutExtraTrackPID_

