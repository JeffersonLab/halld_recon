// $Id$
//
// File: JEventProcessor_BCAL_Eff.h
// Created: Fri Oct 10 16:41:18 EDT 2014
// Creator: wmcginle (on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_BCAL_Eff_
#define _JEventProcessor_BCAL_Eff_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include <ANALYSIS/DHistogramActions.h>
#include "ANALYSIS/DAnalysisUtilities.h"
#include "TRACKING/DTrackFitter.h"

#include "DLorentzVector.h"
#include "TMatrixD.h"


using namespace std;

class JEventProcessor_BCAL_Eff : public JEventProcessor
{
	public:
		JEventProcessor_BCAL_Eff(){
			SetTypeName("JEventProcessor_BCAL_Eff");
		};
		~JEventProcessor_BCAL_Eff(){};

		uint32_t BCALShowers_per_event  ;

		int Run_Number;


	private:
//		const DAnalysisUtilities* dAnalysisUtilities;
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

};

#endif // _JEventProcessor_BCAL_Eff_

