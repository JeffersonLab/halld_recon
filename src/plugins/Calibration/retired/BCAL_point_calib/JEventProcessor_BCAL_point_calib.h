// $Id$
//
//    File: JEventProcessor_BCAL_point_calib.h
// Created: Mon Sep 26 09:38:23 EDT 2016
// Creator: gvasil (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_BCAL_point_calib_
#define _JEventProcessor_BCAL_point_calib_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DHistogramActions.h>
#include "ANALYSIS/DAnalysisUtilities.h"
#include "TRACKING/DTrackFitter.h"
#include <BCAL/DBCALGeometry.h>

#include "DLorentzVector.h"
#include "TMatrixD.h"


using namespace std;

class JEventProcessor_BCAL_point_calib:public JEventProcessor{
	public:
		JEventProcessor_BCAL_point_calib();
		~JEventProcessor_BCAL_point_calib();

	private:
//		const DAnalysisUtilities* dAnalysisUtilities;
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;


		uint32_t BCALShowers_per_event;
		int Run_Number;
		const DBCALGeometry *dBCALGeom;

		bool DEBUG;    // control the creation of extra histograms
		bool VERBOSE;  // verbose output

};

#endif // _JEventProcessor_BCAL_point_calib_

