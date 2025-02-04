// $Id$
//
//    File: JEventProcessor_BCAL_attenlength_gainratio.h
// Created: Mon Aug 10 10:17:48 EDT 2015
// Creator: dalton (on Linux gluon02.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_BCAL_attenlength_gainratio_
#define _JEventProcessor_BCAL_attenlength_gainratio_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include "BCAL/DBCALGeometry.h"
#include "TH2.h"

class JEventProcessor_BCAL_attenlength_gainratio:public JEventProcessor{
	public:
		JEventProcessor_BCAL_attenlength_gainratio();
		~JEventProcessor_BCAL_attenlength_gainratio();

		static const int nummodule=48;
		static const int numlayer=4;
		static const int numsector=4;

		double z_target_center;

	private:
		uint32_t VERBOSE;
		uint32_t VERBOSEHISTOGRAMS;

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;


		//DO NOT MAKE THESE STATIC GLOBAL EVER AGAIN!!

		// Summary histograms
		TH1I *hist_attenlength = nullptr;
		TH1I *hist_gainratio = nullptr;
		TH1I *hist_attenlength_err = nullptr;
		TH1I *hist_gainratio_err = nullptr;
		TH1I *hist_attenlength_relerr = nullptr;
		TH1I *hist_gainratio_relerr = nullptr;
		TH2F *hist2D_peakattenlength = nullptr;
		TH2F *hist2D_peakgainratio = nullptr;
		TH2F *hist2D_intattenlength = nullptr;
		TH2F *hist2D_intgainratio = nullptr;

		// Channel by channel histograms
		TH2I *logpeakratiovsZ_all = nullptr;
		TH2I *logintratiovsZ_all = nullptr;
		TH2I *logpeakratiovsZ[JEventProcessor_BCAL_attenlength_gainratio::nummodule][JEventProcessor_BCAL_attenlength_gainratio::numlayer][JEventProcessor_BCAL_attenlength_gainratio::numsector];
		TH2I *logintratiovsZ[JEventProcessor_BCAL_attenlength_gainratio::nummodule][JEventProcessor_BCAL_attenlength_gainratio::numlayer][JEventProcessor_BCAL_attenlength_gainratio::numsector];
		TH2I *logEratiovsZ[JEventProcessor_BCAL_attenlength_gainratio::nummodule][JEventProcessor_BCAL_attenlength_gainratio::numlayer][JEventProcessor_BCAL_attenlength_gainratio::numsector];
		TH2I *EvsZ[JEventProcessor_BCAL_attenlength_gainratio::nummodule][JEventProcessor_BCAL_attenlength_gainratio::numlayer][JEventProcessor_BCAL_attenlength_gainratio::numsector];

		// Debug histograms to help understand data
		TH2I *EvsZ_all = nullptr;
		TH2I *EvsZ_layer[4] = { nullptr };
		TH2I *logintratiovsZ_layers[4] = { nullptr };
		TH2I *logEratiovsZ_layers[4] = { nullptr };
		TH2F *hist2D_aveZ = nullptr;
};

#endif // _JEventProcessor_BCAL_attenlength_gainratio_

