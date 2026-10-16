
// $Id$
//
//    File: DEventProcessor_ccal_display.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_ccal_display_
#define _DEventProcessor_ccal_display_

#include <JANA/JEventProcessor.h>

#include "TApplication.h"
#include "TCanvas.h"

#include "TProfile.h"
#include <TTree.h>


#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

#include "RootHeader.h"

class DEventProcessor_ccal_display:public JEventProcessor{
	public:
		DEventProcessor_ccal_display(){
            SetTypeName("DEventProcessor_ccal_display");
		};
		~DEventProcessor_ccal_display(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		

                TApplication *app1;
                TCanvas *my_canvas;

		TProfile *ccal_wave[144];

};

#endif // _DEventProcessor_ccal_display_

