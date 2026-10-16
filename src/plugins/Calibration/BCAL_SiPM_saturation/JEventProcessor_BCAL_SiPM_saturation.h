// $Id$
//
//    File: JEventProcessor_BCAL_SiPM_saturation.h
//          Modified file from BCAL_neutron_discriminator.h   ES 5/10/2018
// Created: Thu Apr  5 16:36:00 EDT 2018
// Creator: dalton (on Linux gluon119.jlab.org 2.6.32-642.3.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_BCAL_SiPM_saturation_
#define _JEventProcessor_BCAL_SiPM_saturation_

#include <JANA/JEventProcessor.h>

#include "TH1F.h"
#include "TH2F.h"

using std::vector;

class JEventProcessor_BCAL_SiPM_saturation:public JEventProcessor{
	public:
		JEventProcessor_BCAL_SiPM_saturation();
		~JEventProcessor_BCAL_SiPM_saturation();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		uint32_t VERBOSE;


		vector< vector<double> > attenuation_parameters;    // store attenuation length
		
		TH1F *dHistEthrown;
    	TH1F *dHistEshower;
    	TH1F *dHistThrownTheta;
    	TH1F *dHistNCell;
    	TH1F *dHistLayer;
    	TH1F *dHistEpoint;

    	TH1F *dHistPeakLayer1;
    	TH1F *dHistIntegralLayer1;
    	TH1F *dHistPeakLayer2;
    	TH1F *dHistIntegralLayer2;
    	TH1F *dHistPeakLayer3;
    	TH1F *dHistIntegralLayer3;
    	TH1F *dHistPeakLayer4;
    	TH1F *dHistIntegralLayer4;
    	TH1F *dHistEcalc;
    	TH1F *dHistEcalcEpt;


    	TH2F *dHistEshowerVsEthrown;
    	TH2F *dHistEdiffVsEthrown;
    	TH2F *dHistEdiffFracVsEthrown;
    	
};

#endif // _JEventProcessor_BCAL_SiPM_saturation_

