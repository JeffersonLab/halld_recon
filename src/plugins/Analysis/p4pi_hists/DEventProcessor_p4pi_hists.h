// $Id$
//
//    File: DEventProcessor_p4pi_hists.h
// Created: Mon Aug 29 16:20:58 EDT 2016
// Creator: aaustreg (on Linux halld01.jlab.org 2.6.32-642.3.1.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_p4pi_hists_
#define _DEventProcessor_p4pi_hists_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_p4pi_hists.h"

using namespace std;

class DEventProcessor_p4pi_hists : public JEventProcessor
{
	public:
		DEventProcessor_p4pi_hists() {
			SetTypeName("DEventProcessor_p4pi_hists");
		}
		~DEventProcessor_p4pi_hists() {}

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		//For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here

		//For making EventStore skims
		int Get_FileNumber(const std::shared_ptr<const JEvent>& locEvent) const;
		ofstream dEventStoreSkimStream;
};

#endif // _DEventProcessor_p4pi_hists_

