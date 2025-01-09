// $Id$
//
//    File: DCustomAction_HistOmegaVsMissProton.h
// Created: Sun Jun 28 22:48:32 EDT 2015
// Creator: pmatt (on Darwin Pauls-MacBook-Pro-2.local 13.4.0 i386)
//

#ifndef _DCustomAction_HistOmegaVsMissProton_
#define _DCustomAction_HistOmegaVsMissProton_

#include <string>
#include <iostream>

#include <JANA/JEvent.h>
#include "JANA/JApplication.h"

#include "ANALYSIS/DAnalysisAction.h"
#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DParticleCombo.h"
#include "ANALYSIS/DAnalysisUtilities.h"

#include "TH2I.h"

using namespace std;

class DCustomAction_HistOmegaVsMissProton : public DAnalysisAction
{
	public:

		DCustomAction_HistOmegaVsMissProton(const DReaction* locReaction, string locActionUniqueString = "") : 
		DAnalysisAction(locReaction, "Custom_HistOmegaVsMissProton", false, locActionUniqueString) {}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent) { locEvent->GetSingle(dAnalysisUtilities); }

	private:

		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo);

		// Optional: Useful utility functions.
		const DAnalysisUtilities* dAnalysisUtilities;

		//Store any histograms as member variables here
		TH2I* dHist_OmegaVsMissProton;
};

#endif // _DCustomAction_HistOmegaVsMissProton_

