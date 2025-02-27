// $Id$
//
//    File: DEventProcessor_b1pi_hists.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: pmatt (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_b1pi_hists_
#define _DEventProcessor_b1pi_hists_

#include "TFile.h"
#include "TROOT.h"

#include "JANA/JEventProcessor.h"

#include "ANALYSIS/DAnalysisResults.h"
#include "ANALYSIS/DEventWriterROOT.h"
#include "HDDM/DEventWriterREST.h"

#include "DFactoryGenerator_b1pi_hists.h"


class DEventProcessor_b1pi_hists : public JEventProcessor
{
	public:
		DEventProcessor_b1pi_hists(){
            SetTypeName("DEventProcessor_b1pi_hists");
		};
		~DEventProcessor_b1pi_hists(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};


#endif // _DEventProcessor_b1pi_hists_

