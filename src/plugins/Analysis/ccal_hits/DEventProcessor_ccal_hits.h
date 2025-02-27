// $Id$
//
//    File: DEventProcessor_ccal_hits.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_ccal_hits_
#define _DEventProcessor_ccal_hits_

#include <JANA/JEventProcessor.h>

#include "TApplication.h"
#include "TCanvas.h"

#include "TProfile.h"
#include <TTree.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>


class DEventProcessor_ccal_hits:public JEventProcessor{
	public:
		DEventProcessor_ccal_hits(){
            SetTypeName("DEventProcessor_ccal_hits");
		};
		~DEventProcessor_ccal_hits(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		

		TTree* tree1;

		static const Int_t max_hit     =  3000;
		static const Int_t max_sample  =  100;

		
		Int_t nhit;
		Int_t column[max_hit];
		Int_t row[max_hit];
		Int_t peak[max_hit];
		Int_t integral[max_hit];
		Int_t pedestal[max_hit];
		Int_t time[max_hit];
		Int_t qf[max_hit];

		Int_t waveform[max_hit][max_sample];

		Int_t nsamples_integral;
		Int_t nsamples_pedestal;

		TProfile *ccal_wave[144];
		TH1F *ccal_peak[144];
		TH1F *ccal_int[144];
		
};

#endif // _DEventProcessor_ccal_hits_

