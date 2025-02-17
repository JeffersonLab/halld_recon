// $Id$
//
//    File: JEventProcessor_syncskim.h
// Created: Wed Feb 22 20:04:25 EST 2017
// Creator: davidl (on Linux gluon48.jlab.org 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_syncskim_
#define _JEventProcessor_syncskim_

#include <atomic>

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include <TFile.h>
#include <TTree.h>
#include "SyncEvent.h"
#include "ConversionParms.h"

class JEventProcessor_syncskim:public JEventProcessor{
	public:
		JEventProcessor_syncskim();
		~JEventProcessor_syncskim();

		SyncEvent synevt;
		ConversionParms convparms;
		TFile *file;
		TTree *tree;
		TTree *conversion_tree;
		
		uint32_t SYNCSKIM_ROCID;

		// Values to do linear regression to find slope and intercept correlating
		// 250MHz clock time to unix timestamp
		double sum_n;
		double sum_x;
		double sum_y;
		double sum_xy;
		double sum_x2;
		
		// Some TAC runs were taken without sync events. For these we use the 
		// CODA control events as a backup
		std::atomic<double> last_control_event_t;
		std::atomic<double> last_physics_event_t;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_syncskim_

