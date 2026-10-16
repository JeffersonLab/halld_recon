// $Id$
//
//    File: DCustomAction_RecoilMass.h
// Created: Wed Jun 19 17:20:17 EDT 2019
// Creator: jrsteven (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DCustomAction_RecoilMass_
#define _DCustomAction_RecoilMass_

#include <string>
#include <iostream>

#include <JANA/JEvent.h>
#include "JANA/JApplication.h"

#include "ANALYSIS/DAnalysisAction.h"
#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DParticleCombo.h"
#include "ANALYSIS/DAnalysisUtilities.h"

using namespace std;

class DCustomAction_RecoilMass : public DAnalysisAction
{
	public:

                DCustomAction_RecoilMass(const DReaction* locReaction, bool locUseKinFitResultsFlag, deque<int> locRecoilIndices, double locMinRecoilMass, double locMaxRecoilMass, string locActionUniqueString = "") : 
	        DAnalysisAction(locReaction, "Custom_RecoilMass", locUseKinFitResultsFlag, locActionUniqueString), dRecoilIndices(locRecoilIndices), dMinRecoilMass(locMinRecoilMass), dMaxRecoilMass(locMaxRecoilMass) {}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent) {};

		void Reset_NewEvent(void){}; //RESET HISTOGRAM DUPLICATE-CHECK TRACKING HERE!!
	private:

		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo);

		//Store any histograms as member variables here
		deque<int> dRecoilIndices;
		double dMinRecoilMass, dMaxRecoilMass;
		
		TH1F *dHistRecoilMass;
};

#endif // _DCustomAction_RecoilMass_

