// $Id$
//
//    File: JEventProcessor_CCAL_online.h
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_CCAL_online_
#define _JEventProcessor_CCAL_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include <stdint.h>
#include <vector>
#include <iostream>

#include <JANA/JApplication.h>
#include <DANA/DEvent.h>
#include <DANA/DStatusBits.h>

#include <CCAL/DCCALDigiHit.h>
#include <CCAL/DCCALHit.h>
#include <CCAL/DCCALGeometry.h>
#include <CCAL/DCCALShower.h>

#include <FCAL/DFCALShower.h>
#include <FCAL/DFCALGeometry.h>

#include <PID/DBeamPhoton.h>
#include <PID/DEventRFBunch.h>
#include <HDGEOMETRY/DGeometry.h>

#include <TRIGGER/DL1Trigger.h>
#include <TRIGGER/DTrigger.h>

#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulsePedestal.h>
#include <DAQ/Df250PulseData.h>

#include "units.h"
#include "DLorentzVector.h"
#include "DVector3.h"

#include "TDirectory.h"
#include "TH2F.h"
#include "TH1I.h"
#include "TH2I.h"
#include "TProfile.h"
#include "TProfile2D.h"

using namespace std;


class JEventProcessor_CCAL_online:public JEventProcessor
{
	
	public:
  		JEventProcessor_CCAL_online();
  		~JEventProcessor_CCAL_online() {};

 	private:

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

		int ccalLayer( int, int );
		
		double BEAM_RF_MAIN_PEAK;
		
		const double m_e = 0.510998928e-3;
		const double c   = 29.9792458;
		
		double m_beamX = 0.;
		double m_beamY = 0.;
		double m_beamZ;
		
		TH1D *ccal_num_events;
		
		TH1I *hdigN;
		TH2F *hdigOcc2D;
		TH1I *hdigInt;
		TH1I *hdigPeak;
		TH1I *hdigT;
		TH1I *hdigPed;
		TProfile *hdigPedChan;
		TH2F *hdigPed2D;
		TH2F *hdigPedSq2D;
		TH2I *hdigIntVsPeak;
		TH1I *hdigQF;
		
		TH1I *hhitN;
		TH1I *hhitE;
		TH1I *hhitETot;
		TH1I *hhitiop;
		TH1I *hhitT;
		TH2F *hhitE2D;
		TH2F *hhitOcc2D;
		
		TH1I *hshowN;
		TH1I *hshowE;
		TH1I *hshowETot;
		TH1I *hshowT;
		TH1I *hshowDime;
		TH2I *hshowXYHigh;
		TH2I *hshowXYLow;
		TH1I *hshowPhi;
		TH1I *hshow2GMass;
		TH1I *hshow2GMass_fcal;
		TH2I *hshowOccEmax;
  		
		TH1F *hcomp_bfdt;
		TH1F *hcomp_fcdt;
		TH1F *hcomp_bcdt;
		
		TH1F *hcomp_cratio;
		TH1F *hcomp_cfbratio;
		TH2F *hcomp_cfb2d;
		TH1F *hcomp_pfpc;
		TH2F *hcomp_cxy;
		TH2F *hcomp_fxy;
		
		TH1F *hcomp_cratio_bkgd;
		TH1F *hcomp_cfbratio_bkgd;
		TH2F *hcomp_cfb2d_bkgd;
		TH2F *hcomp_cxy_bkgd;
		TH2F *hcomp_fxy_bkgd;
		
		TH1I *hNPhotons;
	
};

#endif // _JEventProcessor_CCAL_online_

