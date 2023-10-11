// $Id$
//
//    File: DEventProcessor_B3pi_eff_missgamma.h
// Created: Fri Jun 30 00:38:23 EDT 2017
// Creator: jmhardin (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DEventProcessor_B3pi_eff_missgamma_
#define _DEventProcessor_B3pi_eff_missgamma_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_B3pi_eff_missgamma.h"

using namespace std;

class DEventProcessor_B3pi_eff_missgamma : public JEventProcessor
{
	public:
		DEventProcessor_B3pi_eff_missgamma() {
			SetTypeName("DEventProcessor_B3pi_eff_missgamma");
		}
		~DEventProcessor_B3pi_eff_missgamma() override = default;

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

#endif // _DEventProcessor_B3pi_eff_missgamma_

