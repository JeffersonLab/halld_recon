/**************************************************************************                                                             
* HallD software                                                          * 
* Copyright(C) 2022       GlueX and PrimEX-D Collaborations               * 
*                                                                         *                                                                
* Author: The GlueX and PrimEX-D Collaborations                           *                                                                
* Contributors: Igal Jaegle                                               *                                                               
*                                                                         *
*                                                                         *   
* This software is provided "as is" without any warranty.                 *
**************************************************************************/

#ifndef _JEventProcessor_myanalyzer_
#define _JEventProcessor_myanalyzer_

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
#include "PID/DVertex.h"
#include <PID/DNeutralParticle.h>
#include <CCAL/DCCALGeometry.h>
#include <FCAL/DFCALGeometry.h>
#include <FCAL/DFCALCluster.h>
#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALDigiHit.h>
#include <TRIGGER/DL1Trigger.h>
#include <START_COUNTER/DSCHit.h>
#include "TRACKING/DMCThrown.h"
#include <HDGEOMETRY/DGeometry.h>
#include <PID/DChargedTrack.h>
#include "TRACKING/DTrackTimeBased.h"
#include "HistogramTools.h"
#include "PID/DMCReaction.h"
#include <PID/DDetectorMatches.h>

#include "units.h"
#include "DLorentzVector.h"
#include "DVector3.h"
#include "TRandom3.h"

#include "Combination.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;



class JEventProcessor_myanalyzer:public JEventProcessor{
	public:
		JEventProcessor_myanalyzer()  {};
		~JEventProcessor_myanalyzer() {};
		const char* className(void){return "JEventProcessor_myanalyzer";}

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		
		int fcalLayer(int row, int col);
		
		void FillParticleVectors(vector<const DChargedTrack *>&tracks,
					     vector<const DTrackTimeBased *>&pims,
					     vector<const DTrackTimeBased *>&pips);
		
		void Combined6g(vector<const DNeutralParticleHypothesis *>&EMList,
				Double_t &bestChi2Eta,
				Double_t &bestChi2EtaPrim,
				vector<int>&Index6gList,
				vector<int>&Indexprim6gList);
		
		Combination  *combi6;       

		double TargetMass;
		double m_beamX, m_beamY, m_beamZ;
		double m_fcalX, m_fcalY, m_fcalZ;
		double m_ccalX, m_ccalY, m_ccalZ;
		
		const double m_pi0 =  0.1349770;
		const double m_eta =  0.547862;
		const double massBe9 = 8.39479;
		const double massHe4 = 3.727379238;
		
		const double me = 0.510998928e-3;
		const double mp = 0.93827208816;
		const double m_etapr = 0.95778;  
				
		const double FCAL_RF_CUT       =  3.0;
		const double CCAL_RF_CUT       =  3.0;
		const double BCAL_RF_CUT       =  6.012;
		const double TOF_RF_CUT       =  6.5;
		const double BEAM_RF_CUT       =  2.004;
		
		const double MIN_FCAL_ENERGY   =  0.5;
		const double MIN_BEAM_ENERGY   =  8.0;
				
		TH1F * h_trigbit;
		TH1F * h_trig_bit, * h_trig1, * h_trig2, * h_fptrig1, * h_fptrig2;
		TH1F * h_mc_eb;
		TH1F * h_z, * h_r;
		TH1F * h_FCAL_trg_Esum, * h_FCAL_trg1_Esum, * h_FCAL_trg2_Esum, * h_FCAL_trg3_Esum;
		TH2F * h_bcal_e_v_t;
		TH1F * h_fcal_rf, * h_ccal_rf;
		TH2F * h_fcal_xy, * h_fcal_rc, * h_fcal_tof_dx;
		TH1F * h_Esum_bcal, * h_Esum_fcal, * h_Esum_ccal; 
		TH1F * h_Esum_fcal_trg1, * h_Esum_ccal_trg1;
		TH1F * h_Esum_fcal_trg2, * h_Esum_ccal_trg2;
		TH1F * h_Esum_fcal_trg3, * h_Esum_ccal_trg3;
		TH1F * h_mgg_all, * h_mgg_fcal, * h_mgg_fcaltof;
		TH1F * h_mgg, * h_mggpipi;
		TH1F * h_mpipipi, * h_mpipipi_trg2, * h_mpipipi_trg3;
		TH1F * h_mgg_all15;
		TH1F * h_m6g_all, * h_m3pi0_all;
		TH1F * h_m6g_pi0, * h_m3pi0_pi0;
		TH1F * h_m6g_trg2, * h_m3pi0_trg2;
		TH1F * h_m6g_trg3, * h_m3pi0_trg3;
		TH1F * h_m2g_sc, * h_m2g_sc_trg2, * h_m2g_sc_trg3, * h_m2g_sc_w, * h_m2g_sc_w_trg2, * h_m2g_sc_w_trg3;
		TH1F * h_m6g_sc, * h_m6g_sc_trg2, * h_m6g_sc_trg3, * h_m6g_sc_w, * h_m6g_sc_w_trg2, * h_m6g_sc_w_trg3; 
		TH1F * h_m2g2pi_sc, * h_m2g2pi_sc_trg2, * h_m2g2pi_sc_trg3, * h_m2g2pi_sc_w, * h_m2g2pi_sc_w_trg2, * h_m2g2pi_sc_w_trg3;
		TH1F * h_FCAL2g_trg_Esum_sc, * h_FCAL2g_trg1_Esum_sc, * h_FCAL2g_trg2_Esum_sc, * h_FCAL2g_trg3_Esum_sc, 
		  * h_FCAL2g_trg_Esum_sc_w, * h_FCAL2g_trg1_Esum_sc_w, * h_FCAL2g_trg2_Esum_sc_w, * h_FCAL2g_trg3_Esum_sc_w;
		TH1F * h_FCAL3pi0_trg_Esum_sc, * h_FCAL3pi0_trg1_Esum_sc, * h_FCAL3pi0_trg2_Esum_sc, * h_FCAL3pi0_trg3_Esum_sc, 
		  * h_FCAL3pi0_trg_Esum_sc_w, * h_FCAL3pi0_trg1_Esum_sc_w, * h_FCAL3pi0_trg2_Esum_sc_w, * h_FCAL3pi0_trg3_Esum_sc_w;
		TH1F * h_FCAL2g2pi_trg_Esum_sc, * h_FCAL2g2pi_trg1_Esum_sc, * h_FCAL2g2pi_trg2_Esum_sc, * h_FCAL2g2pi_trg3_Esum_sc, 
		  * h_FCAL2g2pi_trg_Esum_sc_w, * h_FCAL2g2pi_trg1_Esum_sc_w, * h_FCAL2g2pi_trg2_Esum_sc_w, * h_FCAL2g2pi_trg3_Esum_sc_w;
		TH1F * h_FCALetato2g_trg_Esum_sc, * h_FCALetato2g_trg1_Esum_sc, * h_FCALetato2g_trg2_Esum_sc, * h_FCALetato2g_trg3_Esum_sc, 
		  * h_FCALetato2g_trg_Esum_sc_w, * h_FCALetato2g_trg1_Esum_sc_w, * h_FCALetato2g_trg2_Esum_sc_w, * h_FCALetato2g_trg3_Esum_sc_w;
		TH1F * h_FCALetato3pi0_trg_Esum_sc, * h_FCALetato3pi0_trg1_Esum_sc, * h_FCALetato3pi0_trg2_Esum_sc, * h_FCALetato3pi0_trg3_Esum_sc, 
		  * h_FCALetato3pi0_trg_Esum_sc_w, * h_FCALetato3pi0_trg1_Esum_sc_w, * h_FCALetato3pi0_trg2_Esum_sc_w, * h_FCALetato3pi0_trg3_Esum_sc_w;
		TH1F * h_FCALetato2g2pi_trg_Esum_sc, * h_FCALetato2g2pi_trg1_Esum_sc, * h_FCALetato2g2pi_trg2_Esum_sc, * h_FCALetato2g2pi_trg3_Esum_sc, 
		  * h_FCALetato2g2pi_trg_Esum_sc_w, * h_FCALetato2g2pi_trg1_Esum_sc_w, * h_FCALetato2g2pi_trg2_Esum_sc_w, * h_FCALetato2g2pi_trg3_Esum_sc_w;
		TH1F * h_theta_2g, * h_theta_6g, * h_theta_2g2pi;
		TH2F * h_Primakoff_2g, * h_Primakoff_2g_r;
		TH2F * h_Primakoff_6g, * h_Primakoff_6g_r;
		TH2F * h_Primakoff_2g2pi, * h_Primakoff_2g2pi_r;
		TH2F * h_tagh, * h_tagm;
		TH2F * h_TaggerTiming_vs_eg, * h_TaggerTiming_vs_egcut;
		
};

#endif // _JEventProcessor_myanalyzer_

