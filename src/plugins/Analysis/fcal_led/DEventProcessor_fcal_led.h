// $Id$
//
//    File: DEventProcessor_fcal_led.h

#ifndef _DEventProcessor_fcal_led_
#define _DEventProcessor_fcal_led_

#include <JANA/JEventProcessor.h>
using namespace jana;

#include "TApplication.h"

#include "TProfile.h"
#include <TTree.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>


class DEventProcessor_fcal_led:public JEventProcessor{
 public:
                DEventProcessor_fcal_led(){ 
		  pthread_mutex_init(&mutex, NULL); 
		};

               ~DEventProcessor_fcal_led(){};
		const char* className(void){return "DEventProcessor_fcal_led";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
		
                pthread_mutex_t mutex;

		TTree* tree1;

		static const Int_t max_hit     =  4000;
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

		TProfile *fcal_wave[59][59];
		TH1F *fcal_peak[59][59];
		TH1F *fcal_int[59][59];
		
};

#endif // _DEventProcessor_fcal_led_

