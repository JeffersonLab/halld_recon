// $Id$
//
//    File: JEventProcessor_ST_online_tracking.h
// Created: Fri Jun 19 13:22:21 EDT 2015
// Creator: mkamel (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_ST_online_tracking_
#define _JEventProcessor_ST_online_tracking_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <START_COUNTER/DSCHit.h>
#include <RF/DRFTDCDigiTime.h>
#include <RF/DRFTime_factory.h>
#include <PID/DEventRFBunch.h>
#include <PID/DParticleID.h>
#include <TRACKING/DTrackFitter.h>

#include "TF1.h"
#include "TH1D.h"
#include "TObjArray.h"
#include "TMath.h"
class JEventProcessor_ST_online_tracking:public JEventProcessor{
	public:
		JEventProcessor_ST_online_tracking();
		~JEventProcessor_ST_online_tracking();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

};

#endif // _JEventProcessor_ST_online_tracking_

