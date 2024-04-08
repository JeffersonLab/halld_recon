// $Id$
//
//    File: DEventProcessor_npp_hists.h
// Created: Tue May  3 09:24:43 EDT 2022
// Creator: aaustreg (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DEventProcessor_npp_hists_
#define _DEventProcessor_npp_hists_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_npp_hists.h"

using namespace std;

class DEventProcessor_npp_hists : public JEventProcessor
{
	public:
		const char* className(void){return "DEventProcessor_npp_hists";}

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		//For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here

		//For making EventStore skims
		int Get_FileNumber(const std::shared_ptr<const JEvent>& event) const;
		ofstream dEventStoreSkimStream;
};

#endif // _DEventProcessor_npp_hists_

