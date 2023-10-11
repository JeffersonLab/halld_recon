// $Id$
//
// File: DEventProcessor_fcal_charged.h
// Created: Fri Oct 10 16:41:18 EDT 2014
// Creator: wmcginle (on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_fcal_charged_
#define _DEventProcessor_fcal_charged_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DHistogramActions.h>
#include "ANALYSIS/DAnalysisUtilities.h"
//#include "TRACKING/DTrackFinder.h"

#include "DLorentzVector.h"
#include "TMatrixD.h"


using namespace std;

class DEventProcessor_fcal_charged : public JEventProcessor
{
	public:
		DEventProcessor_fcal_charged(){
			SetTypeName("DEventProcessor_fcal_charged");
		};
		~DEventProcessor_fcal_charged(){};

	private:
		const DAnalysisUtilities* dAnalysisUtilities;
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		TMatrixD m_mC;
		TMatrixD m_mD;
		TMatrixD m_nhits;
		
		TH2F* h2D_mC;
		TH1F* h1D_mD;
		TH1F* h1D_nhits;
	   

};

#endif // _DEventProcessor_fcal_charged_

