// $Id$
//
//    File: DEventProcessor_bcal_hists.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_fcal_hists_
#define _DEventProcessor_fcal_hists_

#include <JANA/JEventProcessor.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

class DEventProcessor_fcal_hists:public JEventProcessor{
	public:
		DEventProcessor_fcal_hists(){
			SetTypeName("DEventProcessor_fcal_hists");
		};
		~DEventProcessor_fcal_hists(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		
		TH2D *dE_over_E_vs_E;
};

#endif // _DEventProcessor_fcal_hists_

