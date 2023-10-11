// $Id$
//
//    File: JEventProcessor_ST_online_multi.h
// Created: Wed Feb 24 08:17:21 EST 2016
// Creator: mkamel (on Linux mkamel-NE56R 3.13.0-39-generic x86_64)
//

#ifndef _JEventProcessor_ST_online_multi_
#define _JEventProcessor_ST_online_multi_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
using namespace std;
// ***************** C++ header files******************
//*****************************************************
#include <stdint.h>
#include <vector>
#include <stdio.h>
//***************** ROOT header files********************
//*******************************************************
#include <TMath.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
//***************** ST header files************************
//*********************************************************
#include "START_COUNTER/DSCDigiHit.h"
#include "START_COUNTER/DSCTDCDigiHit.h"
#include "START_COUNTER/DSCHit.h"
//****************************** Define some constants *********
//**************************************************************
const int  NCHANNELS  = 30;     // number of scintillator paddles
class JEventProcessor_ST_online_multi:public JEventProcessor{
	public:
		JEventProcessor_ST_online_multi();
		~JEventProcessor_ST_online_multi();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		std::shared_ptr<JLockService> lockService;
		static const uint32_t ADC_MULTI_MIN  = 0.;      // Lower limit of adc multiplicity histogram
		static const uint32_t ADC_MULTI_MAX  = 51.;     // Upper limit of adc multiplicity histogram
		static const uint32_t ADC_MULTI_BINS = 51.;     // Number of bins in adc multiplicity histogram
		static const uint32_t TDC_MULTI_MIN  = 0.;      // Lower limit of tdc multiplicity histogram
		static const uint32_t TDC_MULTI_MAX  = 70.;     // Upper limit of tdc multiplicity histogram
		static const uint32_t TDC_MULTI_BINS = 70.;     // Number of bins in tdc multiplicity histogram
		double counter_adc[NCHANNELS];
		double counter_tdc[NCHANNELS];
		double counter_hit[NCHANNELS];
		double counter_adc_2[NCHANNELS];
		double counter_tdc_2[NCHANNELS];
		double counter_adc_unmatched[NCHANNELS];
		double counter_tdc_unmatched[NCHANNELS];
		int adc_index;
		int hit_index;
		int tdc_index;

};

#endif // _JEventProcessor_ST_online_multi_

