// $Id$
//
//    File: JEventProcessor_compton_simple.h
// Created: Mon Dec 03 16:04:16 EST 2019
// Creator: andrsmit (on Linux ifarm1402)
//

#ifndef _JEventProcessor_compton_simple_
#define _JEventProcessor_compton_simple_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include "TTree.h"
#include "TH1.h"
#include "TH2.h"

#include <PID/DBeamPhoton.h>
#include <PID/DEventRFBunch.h>


#include <CCAL/DCCALShower.h>
#include <FCAL/DFCALShower.h>
#include <BCAL/DBCALShower.h>
#include <TOF/DTOFPoint.h>

#include <CCAL/DCCALGeometry.h>
#include <FCAL/DFCALGeometry.h>

#include <TRIGGER/DL1Trigger.h>
#include <TRIGGER/DTrigger.h>
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulseData.h>

#include <HDGEOMETRY/DGeometry.h>

#include "units.h"
#include "DLorentzVector.h"
#include "DVector3.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <thread>
#include <mutex>

using namespace jana;
using namespace std;


class JEventProcessor_compton_simple:public JEventProcessor{
 	public:
		JEventProcessor_compton_simple(){};
  		~JEventProcessor_compton_simple(){};
  		const char* className(void){return "JEventProcessor_compton_simple";}
	
 	private:
		jerror_t init(void);
  		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);
  		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);
  		jerror_t erun(void);
  		jerror_t fini(void);
		
		double bar2x( int bar );
		
		
		
		double targetZ;
		
		const double fcalX =  0.529;
		const double fcalY = -0.002;
		double fcalZ;
		
		const double ccalX = -0.029;
		const double ccalY =  0.013;
		double ccalZ;
				
		const double tofX  =  0.308;
		const double tofY  =  0.225;
		double tofZ;
		
		
		
		const double FCAL_BLOCK_SIZE = 4.0157;
		const double CCAL_BLOCK_SIZE = 2.09;
				
		const double c  = 29.9792;
		const double pi	= 3.14159265359;
		const double me = 0.510998928e-3;
		
		
		
		
		// Histograms:
		
		TH1I *h_deltaPhi, *h_deltaT;
		TH1I *h_beam_fcal_dt, *h_beam_ccal_dt;
		
		
		TH1I *h_deltaE,     *h_deltaE_e,     *h_deltaE_g;
		TH1I *h_deltaE_acc, *h_deltaE_e_acc, *h_deltaE_g_acc;
		TH2I *h_deltaE_vs_beamE,     *h_deltaE_vs_beamE_e,     *h_deltaE_vs_beamE_g;
		TH2I *h_deltaE_vs_beamE_acc, *h_deltaE_vs_beamE_e_acc, *h_deltaE_vs_beamE_g_acc;
		
		
		TH1I *h_deltaK_cut,     *h_deltaK_e_cut,     *h_deltaK_g_cut;
		TH1I *h_deltaK_cut_acc, *h_deltaK_e_cut_acc, *h_deltaK_g_cut_acc;
		
		
		TH1I *h_deltaK,     *h_deltaK_e,     *h_deltaK_g;
		TH1I *h_deltaK_acc, *h_deltaK_e_acc, *h_deltaK_g_acc;
		TH2I *h_deltaK_vs_beamE,     *h_deltaK_vs_beamE_e,     *h_deltaK_vs_beamE_g;
		TH2I *h_deltaK_vs_beamE_acc, *h_deltaK_vs_beamE_e_acc, *h_deltaK_vs_beamE_g_acc;
		
		
		TH2I *h_xy_fcal,     *h_xy_ccal;
		TH2I *h_xy_fcal_acc, *h_xy_ccal_acc;
		
		
		
		
		
		
};

#endif // _JEventProcessor_compton_simple_

