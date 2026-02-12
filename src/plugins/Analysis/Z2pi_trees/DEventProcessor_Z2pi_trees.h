// $Id$
// DEventProcessor_Z2pi_trees.h, modeled after DEventProcessor_p2pi_trees.h
//
//    File: DEventProcessor_p2pi_trees.h
// Created: Wed Mar 29 16:34:58 EDT 2017
// Creator: elton (on Linux ifarm1401.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DEventProcessor_Z2pi_trees_
#define _DEventProcessor_Z2pi_trees_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_Z2pi_trees.h"

using namespace std;

class DEventProcessor_Z2pi_trees : public JEventProcessor
{
	public:
		DEventProcessor_Z2pi_trees() {
			SetTypeName("DEventProcessor_Z2pi_trees");
		}

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

#endif // _DEventProcessor_Z2pi_trees_

