// $Id$
//
//    File: JEventProcessor_st_tw_corr_auto.h
// Created: Mon Oct 26 10:35:45 EDT 2015
// Creator: mkamel (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_st_tw_corr_auto_
#define _JEventProcessor_st_tw_corr_auto_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

using namespace std;
using std::vector;
using std::string;
// ***************** C++ header files******************
//*****************************************************
#include <stdint.h>
#include <vector>
#include <stdio.h>
#include <fstream>
// ***************** ST header files*******************************
#include "START_COUNTER/DSCHit.h"
#include "START_COUNTER/DSCDigiHit.h"
// ***************** ROOT header files*****************************
#include <TH1.h>
#include <TH2.h>
#include <TDirectory.h>
#include <TF1.h>
#include <TMath.h>
#include <TFile.h>
// ******************* DAQ libraries******************************
#include <DAQ/Df250PulsePedestal.h>

class JEventProcessor_st_tw_corr_auto:public JEventProcessor{
	public:
		JEventProcessor_st_tw_corr_auto();
		~JEventProcessor_st_tw_corr_auto();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

        
        // ***************** define constants*************************
        Int_t NCHANNELS;
        Double_t tdc_thresh_mV;
        Double_t tdc_gain_factor;
        Double_t adc_max_chan;
        Double_t adc_max_mV;
        Double_t adc_thresh_calc ;
        // ******************* 2D histos **************
        TH2I **h2_stt_vs_pp_chan;
	TH2I ** h2_st_corr_vs_pp;
        // ******************** 1D histos *******************
        TH1I **h_pp_chan;
        TH1I **h_stt_chan;
	TH1I **h1_st_corr_time;
	//Define Calibration parameters variable called from CCDB
	vector<vector<double> >timewalk_parameters;
	double USE_TIMEWALK_CORRECTION;
};

#endif // _JEventProcessor_st_tw_corr_auto_

