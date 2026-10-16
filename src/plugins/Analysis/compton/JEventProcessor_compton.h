/**************************************************************************                                                                                                                           
* HallD software                                                          * 
* Copyright(C) 2019  GlueX and PrimEX-D Collaborations                    * 
*                                                                         *                                                                                                                               
* Author: The GlueX and PrimEX-D Collaborations                           *                                                                                                                                
* Contributors: Andrew Smith                                              *                                                                                                                               
*                                                                         *                                                                                                                               
* This software is provided "as is" without any warranty.                 *
**************************************************************************/
// $Id$
//
//    File: JEventProcessor_compton.h
// Created: Mon Dec 03 16:04:16 EST 2019
// Creator: andrsmit (on Linux ifarm1402)
//

#ifndef _JEventProcessor_compton_
#define _JEventProcessor_compton_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include "TTree.h"
#include "TH1.h"
#include "TH2.h"

#include <PID/DBeamPhoton.h>
#include <PID/DEventRFBunch.h>

#include <CCAL/DCCALShower.h>
#include <FCAL/DFCALShower.h>

#include <CCAL/DCCALGeometry.h>
#include <FCAL/DFCALGeometry.h>
#include <FCAL/DFCALCluster.h>

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

#include "CompCand.h"

using namespace std;


class JEventProcessor_compton:public JEventProcessor{
 	public:
		JEventProcessor_compton(){
			SetTypeName("JEventProcessor_compton");
		};
  		~JEventProcessor_compton(){};

 	private:
		void Init() override;
  		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  		void Process(const std::shared_ptr<const JEvent>& event) override;
  		void EndRun() override;
  		void Finish() override;
		
		int ccalLayer(int row, int col);
		int fcalLayer(int row, int col);
		
		void fill_histograms( vector< ComptonCandidate_t > Comp_Candidates );
		
		
		double m_beamX, m_beamY, m_beamZ;
		double m_fcalX, m_fcalY, m_fcalZ;
		double m_ccalX, m_ccalY, m_ccalZ;
		
		const double c    =  29.9792458;
		const double m_e  =  0.510998928e-3;
		
		
		
		
		//-------------------------------------------------------//
		// Cuts:
		
		const double FCAL_RF_CUT           =  3.0;
		const double CCAL_RF_CUT           =  3.0;
		const double BEAM_RF_CUT           =  2.004;
		
		const double MIN_FCAL_ENERGY_CUT   =  0.5;
		const double MIN_CCAL_ENERGY_CUT   =  1.0;
		const double MIN_BEAM_ENERGY_CUT   =  5.0;
		
		
		
		//-------------------------------------------------------//
		/*
		
		DeltaE Mean and Width Parameters:
		
		mu     =  deltaE_mu_p0 +    deltaE_mu_p1*pow(eb,1.) +    deltaE_mu_p2*pow(eb,2.) 
			               +    deltaE_mu_p3*pow(eb,3.)
		sig/eb = deltaE_sig_p0 + deltaE_sig_p1*pow(eb,-0.5) + deltaE_sig_p2*pow(eb,-1.0)
		
		*/
		
		const double deltaE_mu_p0  =  7.80161e-02;
		const double deltaE_mu_p1  = -2.01908e-02;
		const double deltaE_mu_p2  =  2.39183e-03;
		const double deltaE_mu_p3  = -9.75374e-05;
		
		
		const double deltaE_sig_p0 =  6.49198e-03;
		const double deltaE_sig_p1 =  3.76757e-02;
		const double deltaE_sig_p2 = -1.03981e-02;
		
		
		//-------------------------------------------------------//
		/*
		
		DeltaK Mean and Width Parameters corresponding to 3rd-Order Polynomial:
		
		mu     =  deltaK_mu_p0 +  deltaK_mu_p1*pow(eb,1.) +  deltaK_mu_p2*pow(eb,2.) 
				       +  deltaK_mu_p3*pow(eb,3.)
		sig/eb = deltaK_sig_p0 + deltaK_sig_p1*pow(eb,1.) + deltaK_sig_p2*pow(eb,2.) 
				       + deltaK_sig_p3*pow(eb,3.)
		
		*/
		
		const double deltaK_mu_p0  = -0.237052;
		const double deltaK_mu_p1  =  0.0938042;
		const double deltaK_mu_p2  = -0.0124729;
		const double deltaK_mu_p3  =  0.00046516;
		
		
		const double deltaK_sig_p0 =  0.00315767;
		const double deltaK_sig_p1 =  0.00823311;
		const double deltaK_sig_p2 = -0.000965273;
		const double deltaK_sig_p3 =  3.44084e-05;
		
		//-------------------------------------------------------//
		/*
		
		DeltaPhi Mean and Width Parameters:
		
		mu   =  deltaPhi_mu_p0 +  deltaPhi_mu_p1*pow(eb,-0.5) +  deltaPhi_mu_p2*pow(eb,-1.0)
		sig  = deltaPhi_sig_p0 + deltaPhi_sig_p1*pow(eb,-0.5) + deltaPhi_sig_p2*pow(eb,-1.0)
		*/
		
		const double deltaPhi_mu_p0  =  1.80560e+02;
		const double deltaPhi_mu_p1  = -1.03416e+00;
		const double deltaPhi_mu_p2  =  1.20351e+00;
		
		
		const double deltaPhi_sig_p0 =  8.59459e+00;
		const double deltaPhi_sig_p1 = -1.93291e+01;
		const double deltaPhi_sig_p2 =  3.42598e+01;
		
		
		
		//-------------------------------------------------------//
		// Histograms:
		
		TH1F *hTrig, *hfpTrig;
		
		TH1F *h_fcal_rf_dt,    *h_ccal_rf_dt,    *h_beam_rf_dt;;
		TH1F *h_beam_ccal_dt,  *h_beam_fcal_dt;
		
		
		TH1F *h1_deltaPhi, *h1_deltaPhi_e, *h1_deltaPhi_ek, *h1_deltaPhi_ekp;
		TH2F *h2_deltaPhi, *h2_deltaPhi_e, *h2_deltaPhi_ek, *h2_deltaPhi_ekp;
		
		TH1F *h1_dTheta,   *h1_dTheta_p,   *h1_dTheta_pe,   *h1_dTheta_pek;
		TH2F *h2_dTheta,   *h2_dTheta_p,   *h2_dTheta_pe,   *h2_dTheta_pek;
		
		TH1F *h1_deltaR,   *h1_deltaR_p,   *h1_deltaR_pe,   *h1_deltaR_pek;
		TH2F *h2_deltaR,   *h2_deltaR_p,   *h2_deltaR_pe,   *h2_deltaR_pek;
		
		TH1F *h1_deltaT,   *h1_deltaT_p,   *h1_deltaT_pe,   *h1_deltaT_pek;
		TH2F *h2_deltaT,   *h2_deltaT_p,   *h2_deltaT_pe,   *h2_deltaT_pek;
		
		TH1F *h1_deltaE,          *h1_deltaE_p,          *h1_deltaE_pk;
		TH2F *h2_deltaE,          *h2_deltaE_p,          *h2_deltaE_pk;
		TH1F *h1_deltaE_ring[10], *h1_deltaE_p_ring[10], *h1_deltaE_pk_ring[10];
		TH2F *h2_deltaE_ring[10], *h2_deltaE_p_ring[10], *h2_deltaE_pk_ring[10];
		
		TH2F *h2_deltaE_v_fcalE,  *h2_deltaE_v_fcalE_ring[10];
		TH2F *h2_deltaE_v_ccalE,  *h2_deltaE_v_ccalE_ring[10];
		
		
		TH1F *h1_deltaK,          *h1_deltaK_p,          *h1_deltaK_pe;
		TH2F *h2_deltaK,          *h2_deltaK_p,          *h2_deltaK_pe;
		TH1F *h1_deltaK_ring[10], *h1_deltaK_p_ring[10], *h1_deltaK_pe_ring[10];
		TH2F *h2_deltaK_ring[10], *h2_deltaK_p_ring[10], *h2_deltaK_pe_ring[10];
		
		
		TH2F *h_fcal_xy,          *h_ccal_xy;
		TH2F *h_fcal_xy_ring[10], *h_ccal_xy_ring[10];
		
		
		
		
		
		
};

#endif // _JEventProcessor_compton_

