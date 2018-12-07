// $Id$
//
//    File: DEventProcessor_ccal_hits.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_ccal_hits_
#define _DEventProcessor_ccal_hits_

#include <JANA/JEventProcessor.h>
using namespace jana;

#include "TApplication.h"
#include "TCanvas.h"

#include "TProfile.h"
#include <TTree.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>


class DEventProcessor_ccal_hits:public JEventProcessor{
	public:
		DEventProcessor_ccal_hits(){};
		~DEventProcessor_ccal_hits(){};
		const char* className(void){return "DEventProcessor_ccal_hits";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
		

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

