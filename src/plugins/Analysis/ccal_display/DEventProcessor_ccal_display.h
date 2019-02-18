
// $Id$
//
//    File: DEventProcessor_ccal_display.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_ccal_display_
#define _DEventProcessor_ccal_display_

#include <JANA/JEventProcessor.h>
using namespace jana;

#include "TApplication.h"
#include "TCanvas.h"

#include "TProfile.h"
#include <TTree.h>


#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

#include <RootHeader.h>

class DEventProcessor_ccal_display:public JEventProcessor{
	public:
		DEventProcessor_ccal_display(){};
		~DEventProcessor_ccal_display(){};
		const char* className(void){return "DEventProcessor_ccal_display";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
		

                TApplication *app1;
                TCanvas *my_canvas;

		TProfile *ccal_wave[144];

};

#endif // _DEventProcessor_ccal_display_

