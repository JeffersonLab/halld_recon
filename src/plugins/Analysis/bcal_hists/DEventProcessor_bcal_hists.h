// $Id$
//
//    File: DEventProcessor_bcal_hists.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_bcal_hists_
#define _DEventProcessor_bcal_hists_

#include <JANA/JEventProcessor.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

class DEventProcessor_bcal_hists:public JEventProcessor{
	public:
		DEventProcessor_bcal_hists(){
			SetTypeName("DEventProcessor_bcal_hists");
		};
		~DEventProcessor_bcal_hists(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		
		TH1F* two_gamma_mass, *two_gamma_mass_corr, *two_gamma_mass_cut;
		TH1F* bcal_fcal_two_gamma_mass, *bcal_fcal_two_gamma_mass_cut;
		TH2F* xy_shower;
		TH1F* z_shower;
		TH1F* E_shower;
		TH2F* Erec_over_Ethrown_vs_z;
		TH2F* Ecorr_over_Erec_vs_z;
		TH2F* Ereconstructed_vs_Ethrown;
		TH1F* Etot_truth, *Etot_hits;
		TH2F* Etruth_over_Ethrown_vs_z;
		TH2F *Edeposited_over_Ethrown_vs_z;
};

#endif // _DEventProcessor_bcal_hists_

