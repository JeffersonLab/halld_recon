// $Id$
//
//    File: JEventProcessor_BCAL_TimeCalibration.h
// Created: Mon Apr 18 15:28:52 CST 2016
// Creator: semenov (on Linux selene.phys.uregina.ca 2.6.32-573.18.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_BCAL_TimeCalibration_
#define _JEventProcessor_BCAL_TimeCalibration_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>
#include "ANALYSIS/DAnalysisUtilities.h"
//#include "TRACKING/DTrackFinder.h"

#include "DLorentzVector.h"
#include "TMatrixD.h"


using namespace std;

class JEventProcessor_BCAL_TimeCalibration:public JEventProcessor{
	public:
	
		JEventProcessor_BCAL_TimeCalibration();
		~JEventProcessor_BCAL_TimeCalibration();
		SetTypeName("JEventProcessor_BCAL_TimeCalibration");

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _JEventProcessor_BCAL_TimeCalibration_