// $Id$
//
//    File: DCustomAction_cpp_hists.h
// Created: Thu May 12 12:29:15 EDT 2022
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DCustomAction_cpp_hists_
#define _DCustomAction_cpp_hists_

#include <string>
#include <iostream>

#include "JANA/JEventLoop.h"
#include "JANA/JApplication.h"

#include "ANALYSIS/DAnalysisAction.h"
#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DParticleCombo.h"
#include "ANALYSIS/DAnalysisUtilities.h"

using namespace std;
using namespace jana;

class DCustomAction_cpp_hists : public DAnalysisAction
{
	public:

		DCustomAction_cpp_hists(const DReaction* locReaction, bool locUseKinFitResultsFlag, string locActionUniqueString = "") : 
		DAnalysisAction(locReaction, "Custom_cpp_hists", locUseKinFitResultsFlag, locActionUniqueString) {}

		void Initialize(JEventLoop* locEventLoop);
		void Run_Update(JEventLoop* locEventLoop);
	private:

		bool Perform_Action(JEventLoop* locEventLoop, const DParticleCombo* locParticleCombo);
		
		// Parameters for event selection to fill histograms
		int endpoint_energy_bins;
		double cohmin_energy, cohedge_energy, endpoint_energy;

		// Optional: Useful utility functions.
		const DAnalysisUtilities* dAnalysisUtilities;

                TH1I *dEgamma, *dcosTheta_rho, *dphi_rho, *dPhi_rho, *dpsi_rho, *dt_rho, *dcosTheta_cpp, *dphi_cpp, *dPhi_cpp, *dpsi_cpp, *dt_cpp;
		TH2I *dpsi_vs_E_rho;

		//Store any histograms as member variables here
};

#endif // _DCustomAction_cpp_hists_

