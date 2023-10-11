// $Id$
//
//    File: DEventProcessor_run_summary.h
// Created: Tue Nov 18 15:44:17 EST 2014
// Creator: sdobbs (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_run_summary_
#define _DEventProcessor_run_summary_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include <TTree.h>

#include "DEPICSstore.h"

///#include "DFactoryGenerator_run_summary.h"

using namespace std;

class DEventProcessor_run_summary : public JEventProcessor
{
	public:
		DEventProcessor_run_summary() {
			SetTypeName("DEventProcessor_run_summary");
		}
		~DEventProcessor_run_summary() override = default;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

		int current_run_number;
		TTree *conditions_tree;
		DEPICSstore *epics_info;
};

#endif // _DEventProcessor_run_summary_

