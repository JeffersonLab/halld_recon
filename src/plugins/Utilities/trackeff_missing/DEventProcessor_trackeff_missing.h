// $Id$
//
//    File: DEventProcessor_trackeff_missing.h
// Created: Wed Feb 25 08:58:19 EST 2015
// Creator: pmatt (on Linux pmattdesktop.jlab.org 2.6.32-504.8.1.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_trackeff_missing_
#define _DEventProcessor_trackeff_missing_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_trackeff_missing.h"

using namespace std;

class DEventProcessor_trackeff_missing : public JEventProcessor
{
	public:
		DEventProcessor_trackeff_missing() {
			SetTypeName("DEventProcessor_trackeff_missing");
		}
		~DEventProcessor_trackeff_missing() override = default;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		//For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here
};

#endif // _DEventProcessor_trackeff_missing_

