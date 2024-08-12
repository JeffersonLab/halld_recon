// $Id$
//
//    File: DCustomAction_MissingMatch.h
// Created: Wed Jun 19 17:20:17 EDT 2019
// Creator: jrsteven (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DCustomAction_MissingMatch_
#define _DCustomAction_MissingMatch_

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

class DCustomAction_MissingMatch : public DAnalysisAction
{
	public:

        DCustomAction_MissingMatch(const DReaction* locReaction, bool locUseKinFitResultsFlag, deque<int> locChargedIndices, string locActionUniqueString = "") : 
	DAnalysisAction(locReaction, "Custom_MissingMatch", locUseKinFitResultsFlag, locActionUniqueString), dChargedIndices(locChargedIndices) {}

		void Initialize(JEventLoop* locEventLoop);
		void Run_Update(JEventLoop* locEventLoop) {
			const DParticleID* locParticleID = NULL;
			locEventLoop->GetSingle(locParticleID);
			dParticleID = locParticleID;

			DApplication* dapp=dynamic_cast<DApplication*>(locEventLoop->GetJApplication());
			bfield = dapp->GetBfield((locEventLoop->GetJEvent()).GetRunNumber());
			rt = new DReferenceTrajectory(bfield);
 
			locEventLoop->GetSingle(dAnalysisUtilities); 
		}

		void Reset_NewEvent(void){}; //RESET HISTOGRAM DUPLICATE-CHECK TRACKING HERE!!
	private:

		bool Perform_Action(JEventLoop* locEventLoop, const DParticleCombo* locParticleCombo);

                const DParticleID* dParticleID;
		const DMagneticFieldMap *bfield;
		DReferenceTrajectory *rt;

		const DAnalysisUtilities* dAnalysisUtilities;

		//Store any histograms as member variables here
		deque<int> dChargedIndices;
		
		TH2F *dHistMissingMatch2DTOF, *dHistMissingMatch2DBCAL;
		TH1F *dHistMissingMatchDistTOF, *dHistMissingMatchDistBCAL;
};

#endif // _DCustomAction_MissingMatch_

