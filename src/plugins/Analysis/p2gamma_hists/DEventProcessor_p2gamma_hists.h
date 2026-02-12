// $Id$
//
//    File: DEventProcessor_p2gamma_hists.h
// Created: Tue Apr 28 21:19:41 EDT 2015
// Creator: jrsteven (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_p2gamma_hists_
#define _DEventProcessor_p2gamma_hists_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_p2gamma_hists.h"

using namespace std;

class DEventProcessor_p2gamma_hists : public JEventProcessor
{
	public:
		DEventProcessor_p2gamma_hists() {
			SetTypeName("DEventProcessor_p2gamma_hists");
		}
		~DEventProcessor_p2gamma_hists() {}

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		//For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here
};

#endif // _DEventProcessor_p2gamma_hists_

