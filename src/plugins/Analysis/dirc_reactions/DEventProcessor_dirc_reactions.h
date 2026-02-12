// $Id$
//
//    File: DEventProcessor_dirc_reactions.h
//

#ifndef _DEventProcessor_dirc_reactions_
#define _DEventProcessor_dirc_reactions_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_dirc_reactions.h"

using namespace std;

class DEventProcessor_dirc_reactions : public JEventProcessor
{
	public:
		DEventProcessor_dirc_reactions() {
			SetTypeName("DEventProcessor_dirc_reactions");
		}

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		//For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here
};

#endif // _DEventProcessor_dirc_reactions_

