// $Id$
//
//    File: DEventProcessor_ReactionFilter.h
// Created: Mon Nov 21 17:54:40 EST 2016
// Creator: pmatt (on Darwin Pauls-MacBook-Pro-2.local 13.4.0 i386)
//

#ifndef _DEventProcessor_ReactionFilter_
#define _DEventProcessor_ReactionFilter_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_ReactionFilter.h"

using namespace std;

class DEventProcessor_ReactionFilter : public JEventProcessor
{
	public:
		DEventProcessor_ReactionFilter() {
			SetTypeName("DEventProcessor_ReactionFilter");
		}
		~DEventProcessor_ReactionFilter() override {}

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

#endif // _DEventProcessor_ReactionFilter_

