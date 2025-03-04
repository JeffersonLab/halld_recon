// $Id$
//
//    File: DCustomAction_npp_hists.h
// Created: Thu May 12 12:29:15 EDT 2022
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DCustomAction_npp_hists_
#define _DCustomAction_npp_hists_

#include <string>
#include <iostream>

#include "TLorentzRotation.h"

#include "JANA/JApplication.h"

#include "ANALYSIS/DAnalysisAction.h"
#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DParticleCombo.h"
#include "ANALYSIS/DAnalysisUtilities.h"

using namespace std;

class DCustomAction_npp_hists : public DAnalysisAction
{
	public:

		DCustomAction_npp_hists(const DReaction* locReaction, bool locUseKinFitResultsFlag, string locActionUniqueString = "") : 
		DAnalysisAction(locReaction, "Custom_npp_hists", locUseKinFitResultsFlag, locActionUniqueString) {}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent);
	private:

		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo);
		
		// Parameters for event selection to fill histograms
		int endpoint_energy_bins;
		double cohmin_energy, cohedge_energy, endpoint_energy;

		// Optional: Useful utility functions.
		const DAnalysisUtilities* dAnalysisUtilities;

                TH1I *dEgamma, *dcosTheta, *dphi, *dPhi, *dpsi, *dt; 

		//Store any histograms as member variables here
};

#endif // _DCustomAction_npp_hists_

