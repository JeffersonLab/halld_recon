// $Id$
//
//    File: DCustomAction_p3pi_Pi0Cuts.h
// Created: Thu Jan 22 11:19:46 EST 2015
// Creator: jrsteven (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _DCustomAction_p3pi_Pi0Cuts_
#define _DCustomAction_p3pi_Pi0Cuts_

#include <string>
#include <iostream>

#include <JANA/JEvent.h>
#include "JANA/JApplication.h"

#include "ANALYSIS/DAnalysisAction.h"
#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DParticleCombo.h"
#include "ANALYSIS/DAnalysisUtilities.h"

using namespace std;

class DCustomAction_p3pi_Pi0Cuts : public DAnalysisAction
{
	public:

        DCustomAction_p3pi_Pi0Cuts(const DReaction* locReaction, bool locUseKinFitResultsFlag, double locMinFCAL, double locMinECAL, 
        			string locActionUniqueString = "") : 
	        DAnalysisAction(locReaction, "Custom_p3pi_Pi0Cuts", locUseKinFitResultsFlag, locActionUniqueString), 
	        dMinFCAL(locMinFCAL), dMinECAL(locMinECAL) {}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent) {}

	private:

		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo);

		//Store any histograms as member variables here
		double dMinFCAL;
		double dMinECAL;
		
};

#endif // _DCustomAction_p3pi_Pi0Cuts_

