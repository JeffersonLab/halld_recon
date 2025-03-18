// $Id$
//
//    File: JEventProcessor_F250_mode8_pedestal.h
// Created: Mon Apr 13 14:49:24 EDT 2015
// Creator: dalton (on Linux gluon02.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_F250_mode8_pedestal_
#define _JEventProcessor_F250_mode8_pedestal_

#include <JANA/JEventProcessor.h>
#include "TH1I.h"
#include "TProfile2D.h"

class JEventProcessor_F250_mode8_pedestal:public JEventProcessor{
	public:
		JEventProcessor_F250_mode8_pedestal();
		~JEventProcessor_F250_mode8_pedestal();

		static const int highcratenum=100;
		static const int highslotnum=21;
		static const int highchannum=16;
		// root hist pointers
		TH1I *samples_meansub[highcratenum][highslotnum][highchannum];  /// find the mean in window and them subtract from all samples
		TH1I *samplesum_pedsub[highcratenum][highslotnum][highchannum];  /// measure the pedestal, subtract and then sum the samples
		TH1I *samplesum[highcratenum][highslotnum][highchannum];         /// just sum the samples
		//TH1I *samples_mean[highcratenum][highslotnum][highchannum];
		TProfile2D *mean_crates[highcratenum];
		TH2D *samples_rms_summary[highcratenum];
		TH2D *samplesum_pedsub_rms_summary[highcratenum];
		TH2D *samplesum_rms_summary[highcratenum];


	private:
		uint32_t NSA_NSB;

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _JEventProcessor_F250_mode8_pedestal_

