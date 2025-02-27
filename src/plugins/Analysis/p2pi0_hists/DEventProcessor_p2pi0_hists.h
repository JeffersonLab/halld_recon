// $Id$
//
//    File: DEventProcessor_p2pi0_hists.h
// Created: Tue May 19 07:56:16 EDT 2015
// Creator: jrsteven (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_p2pi0_hists_
#define _DEventProcessor_p2pi0_hists_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_p2pi0_hists.h"

using namespace std;

class DEventProcessor_p2pi0_hists : public JEventProcessor
{
	public:
		DEventProcessor_p2pi0_hists() {
			SetTypeName("DEventProcessor_p2pi0_hists");
		}
		~DEventProcessor_p2pi0_hists() {}

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		//For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here
};

#endif // _DEventProcessor_p2pi0_hists_

