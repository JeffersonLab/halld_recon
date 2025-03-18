// $Id$
//
//    File: DEventProcessor_ReactionEfficiency.h
// Created: Wed Jun 19 16:52:58 EDT 2019
// Creator: jrsteven (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DEventProcessor_ReactionEfficiency_
#define _DEventProcessor_ReactionEfficiency_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_ReactionEfficiency.h"

using namespace std;

class DEventProcessor_ReactionEfficiency : public JEventProcessor
{
	public:
		DEventProcessor_ReactionEfficiency() {
			SetTypeName("DEventProcessor_ReactionEfficiency");
		}
		~DEventProcessor_ReactionEfficiency() {}

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

#endif // _DEventProcessor_ReactionEfficiency_

