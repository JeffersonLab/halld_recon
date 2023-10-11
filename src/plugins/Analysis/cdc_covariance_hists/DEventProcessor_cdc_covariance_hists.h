// $Id$
//
//    File: DEventProcessor_cdc_covariance_hists.h
// Created: Thu Apr 23 08:30:24 EDT 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 Darwin Kernel Version 9.6.0)
//

#ifndef _DEventProcessor_cdc_covariance_hists_
#define _DEventProcessor_cdc_covariance_hists_

#include <pthread.h>
#include <map>
using std::map;

#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile2D.h>

#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>

#include <TRACKING/DReferenceTrajectory.h>
#include <HDGEOMETRY/DMagneticFieldMap.h>

class DEventProcessor_cdc_covariance_hists:public JEventProcessor{

	public:
		DEventProcessor_cdc_covariance_hists();
		~DEventProcessor_cdc_covariance_hists();

		TProfile2D *cdc_cov;
		TProfile2D *cdc_cov_calc;
		
		const DMagneticFieldMap *bfield;
		
	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
			
		pthread_mutex_t mutex;
		
		int Nevents;
		double R_cdc1;
};

#endif // _DEventProcessor_cdc_covariance_hists_

