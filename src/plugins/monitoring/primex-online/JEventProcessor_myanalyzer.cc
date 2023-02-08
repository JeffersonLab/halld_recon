/**************************************************************************                                                             
* HallD software                                                          * 
* Copyright(C) 2022       GlueX Collaboration                             * 
*                                                                         *                                                                
* Author: The GlueX Collaborations                                        *                                                                
* Contributors: Igal Jaegle                                               *                                                               
*                                                                         *
*                                                                         *   
* This software is provided "as is" without any warranty.                 *
**************************************************************************/

#include "JEventProcessor_myanalyzer.h"

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_myanalyzer());
}
} // "C"


//------------------
// init
//------------------
jerror_t JEventProcessor_myanalyzer::init(void)
{
  combi6 = new Combination (6);
  
  TDirectory *main = gDirectory;
  gDirectory->mkdir("primex-online")->cd();
  
  h_trig_bit = new TH1F("trig_bit", ";Trigger bit;Events [a.u.]", 100, 0, 100);
  h_trig1 = new TH1F("trig1", ";GTP trigger bit;Events [a.u.]", 33, -0.5, 32.5);
  h_fptrig1 = new TH1F("fptrig1", ";FP trigger bit;Events [a.u.]", 33, -0.5, 32.5);
  h_trig2 = new TH1F("trig2", ";GTP trigger bit;Events [a.u.]", 33, -0.5, 32.5);
  h_fptrig2 = new TH1F("fptrig2", ";FP trigger bit;Events [a.u.]", 33, -0.5, 32.5);
  
  h_z = new TH1F ("h_z", ";z [cm];Count [a.u.]", 1000, -100., 400.);
  h_r = new TH1F ("h_r", ";r [cm];Count [a.u.]", 1000, 0., 1000.);
  
  h_mc_eb = new TH1F("mc_ev", ";E_{#gamma} [GeV]", 500, 3.0, 12.0);

  h_FCAL_trg_Esum = new TH1F("FCAL_trg_Esum", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  h_FCAL_trg1_Esum = new TH1F("FCAL_trg1_Esum", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  h_FCAL_trg2_Esum = new TH1F("FCAL_trg2_Esum", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  h_FCAL_trg3_Esum = new TH1F("FCAL_trg3_Esum", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  
  h_bcal_e_v_t = new TH2F("bcal_e_v_t", ";t_{#gamma}^{bcal} - t_{RF} [ns];E_{shower} [GeV];count [a.u.]", 2000, -40., 40., 2000, 0., 2.);

  h_fcal_rf = new TH1F("fcal_rf", ";t_{fcal} - t_{RF} [ns];Count [a.u.]", 1000, -50., 50.);
  h_fcal_xy = new TH2F("fcal_xy", ";row;column #;Counts", 500, -125, 125, 500, -125, 125);
  h_fcal_rc = new TH2F("fcal_rc", ";row;column #;Counts", 59, 0, 59, 59, 0, 59);
  h_fcal_tof_dx = new TH2F("fcal_tof_dx", ";#Deltax;#Deltay;Counts", 200, 0, 50, 200, 0, 50);
  
  h_ccal_rf = new TH1F("ccal_rf", ";t_{ccal} - t_{RF} [ns];Count [a.u.]", 1000, -50., 50.);
  
  h_Esum_bcal = new TH1F("Esum_bcal", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  h_Esum_fcal = new TH1F("Esum_fcal", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  h_Esum_ccal = new TH1F("Esum_ccal", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  h_Esum_fcal_trg1 = new TH1F("Esum_fcal_trg1", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  h_Esum_ccal_trg1 = new TH1F("Esum_ccal_trg1", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  h_Esum_fcal_trg2 = new TH1F("Esum_fcal_trg2", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  h_Esum_ccal_trg2 = new TH1F("Esum_ccal_trg2", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  h_Esum_fcal_trg3 = new TH1F("Esum_fcal_trg3", ";E [GeV];Count [a.u.]", 12000, 0., 12.);
  h_Esum_ccal_trg3 = new TH1F("Esum_ccal_trg3", ";E [GeV];Count [a.u.]", 12000, 0., 12.);

  h_mgg_all = new TH1F("mgg_all", ";m_{#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 2000, 0., 2.);
  h_mgg_fcal = new TH1F("mgg_fcal", ";m_{#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 2000, 0., 2.);
  h_mgg_fcaltof = new TH1F("mgg_fcaltof", ";m_{#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 2000, 0., 2.);
  h_mgg = new TH1F("mgg", ";m_{#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 2000, 0., 2.);
  h_mggpipi = new TH1F("mggpipi", ";m_{#gamma#gamma#pi^{+}#pi^{-}} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  h_mpipipi = new TH1F("mpipipi", ";m_{#gamma#gamma#pi^{+}#pi^{-}} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  h_mpipipi_trg2 = new TH1F("mpipipi_trg2", ";m_{#gamma#gamma#pi^{+}#pi^{-}} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  h_mpipipi_trg3 = new TH1F("mpipipi_trg3", ";m_{#gamma#gamma#pi^{+}#pi^{-}} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  
  h_mgg_all15 = new TH1F("mgg_all15", ";m_{#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 2000, 0., 2.);
  h_m6g_all = new TH1F("m6g_all", ";m_{#gamma#gamma#gamma#gamma#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  h_m3pi0_all = new TH1F("m3pi0_all", ";m_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  h_m6g_pi0 = new TH1F("m6g_pi0", ";m_{#gamma#gamma#gamma#gamma#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  h_m3pi0_pi0 = new TH1F("m3pi0_pi0", ";m_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  h_m6g_trg2 = new TH1F("m6g_trg2", ";m_{#gamma#gamma#gamma#gamma#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  h_m3pi0_trg2 = new TH1F("m3pi0_trg2", ";m_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  h_m6g_trg3 = new TH1F("m6g_trg3", ";m_{#gamma#gamma#gamma#gamma#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  h_m3pi0_trg3 = new TH1F("m3pi0_trg3", ";m_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}];Count [a.u.]", 3000, 0., 3.);
  
  h_m2g_sc = new TH1F("m2g_sc", ";m_{#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m2g_sc_trg2 = new TH1F("m2g_sc_trg2", ";m_{#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m2g_sc_trg3 = new TH1F("m2g_sc_trg3", ";m_{#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m2g_sc_w = new TH1F("m2g_sc_w", ";m_{#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m2g_sc_w_trg2 = new TH1F("m2g_sc_w_trg2", ";m_{#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m2g_sc_w_trg3 = new TH1F("m2g_sc_w_trg3", ";m_{#gamma#gamma} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_FCAL2g_trg_Esum_sc = new TH1F("FCAL2g_trg_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g_trg1_Esum_sc = new TH1F("FCAL2g_trg1_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g_trg2_Esum_sc = new TH1F("FCAL2g_trg2_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g_trg3_Esum_sc = new TH1F("FCAL2g_trg3_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g_trg_Esum_sc = new TH1F("FCALetato2g_trg_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g_trg1_Esum_sc = new TH1F("FCALetato2g_trg1_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g_trg2_Esum_sc = new TH1F("FCALetato2g_trg2_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g_trg3_Esum_sc = new TH1F("FCALetato2g_trg3_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g_trg_Esum_sc_w = new TH1F("FCAL2g_trg_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g_trg1_Esum_sc_w = new TH1F("FCAL2g_trg1_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g_trg2_Esum_sc_w = new TH1F("FCAL2g_trg2_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g_trg3_Esum_sc_w = new TH1F("FCAL2g_trg3_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g_trg_Esum_sc_w = new TH1F("FCALetato2g_trg_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g_trg1_Esum_sc_w = new TH1F("FCALetato2g_trg1_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g_trg2_Esum_sc_w = new TH1F("FCALetato2g_trg2_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g_trg3_Esum_sc_w = new TH1F("FCALetato2g_trg3_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  
  h_m6g_sc = new TH1F("m6g_sc", ";m_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m6g_sc_trg2 = new TH1F("m6g_sc_trg2", ";m_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m6g_sc_trg3 = new TH1F("m6g_sc_trg3", ";m_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m6g_sc_w = new TH1F("m6g_sc_w", ";m_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m6g_sc_w_trg2 = new TH1F("m6g_sc_w_trg2", ";m_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m6g_sc_w_trg3 = new TH1F("m6g_sc_w_trg3", ";m_{#pi^{0}#pi^{0}#pi^{0}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_FCAL3pi0_trg_Esum_sc = new TH1F("FCAL3pi0_trg_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL3pi0_trg1_Esum_sc = new TH1F("FCAL3pi0_trg1_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL3pi0_trg2_Esum_sc = new TH1F("FCAL3pi0_trg2_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL3pi0_trg3_Esum_sc = new TH1F("FCAL3pi0_trg3_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato3pi0_trg_Esum_sc = new TH1F("FCALetato3pi0_trg_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato3pi0_trg1_Esum_sc = new TH1F("FCALetato3pi0_trg1_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato3pi0_trg2_Esum_sc = new TH1F("FCALetato3pi0_trg2_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato3pi0_trg3_Esum_sc = new TH1F("FCALetato3pi0_trg3_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL3pi0_trg_Esum_sc_w = new TH1F("FCAL3pi0_trg_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL3pi0_trg1_Esum_sc_w = new TH1F("FCAL3pi0_trg1_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL3pi0_trg2_Esum_sc_w = new TH1F("FCAL3pi0_trg2_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL3pi0_trg3_Esum_sc_w = new TH1F("FCAL3pi0_trg3_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato3pi0_trg_Esum_sc_w = new TH1F("FCALetato3pi0_trg_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato3pi0_trg1_Esum_sc_w = new TH1F("FCALetato3pi0_trg1_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato3pi0_trg2_Esum_sc_w = new TH1F("FCALetato3pi0_trg2_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato3pi0_trg3_Esum_sc_w = new TH1F("FCALetato3pi0_trg3_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
    
  h_m2g2pi_sc = new TH1F("m2g2pi_sc", ";m_{#pi^{0}#pi^{+}#pi^{-}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m2g2pi_sc_trg2 = new TH1F("m2g2pi_sc_trg2", ";m_{#pi^{0}#pi^{+}#pi^{-}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m2g2pi_sc_trg3 = new TH1F("m2g2pi_sc_trg3", ";m_{#pi^{0}#pi^{+}#pi^{-}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m2g2pi_sc_w = new TH1F("m2g2pi_sc_w", ";m_{#pi^{0}#pi^{+}#pi^{-}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m2g2pi_sc_w_trg2 = new TH1F("m2g2pi_sc_w_trg2", ";m_{#pi^{0}#pi^{+}#pi^{-}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_m2g2pi_sc_w_trg3 = new TH1F("m2g2pi_sc_w_trg3", ";m_{#pi^{0}#pi^{+}#pi^{-}} [GeV/#it{c}^{2}];Count [a.u.]", 300, 0., 3.);
  h_FCAL2g2pi_trg_Esum_sc = new TH1F("FCAL2g2pi_trg_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g2pi_trg1_Esum_sc = new TH1F("FCAL2g2pi_trg1_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g2pi_trg2_Esum_sc = new TH1F("FCAL2g2pi_trg2_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g2pi_trg3_Esum_sc = new TH1F("FCAL2g2pi_trg3_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g2pi_trg_Esum_sc = new TH1F("FCALetato2g2pi_trg_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g2pi_trg1_Esum_sc = new TH1F("FCALetato2g2pi_trg1_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g2pi_trg2_Esum_sc = new TH1F("FCALetato2g2pi_trg2_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g2pi_trg3_Esum_sc = new TH1F("FCALetato2g2pi_trg3_Esum_sc", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g2pi_trg_Esum_sc_w = new TH1F("FCAL2g2pi_trg_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g2pi_trg1_Esum_sc_w = new TH1F("FCAL2g2pi_trg1_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g2pi_trg2_Esum_sc_w = new TH1F("FCAL2g2pi_trg2_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCAL2g2pi_trg3_Esum_sc_w = new TH1F("FCAL2g2pi_trg3_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g2pi_trg_Esum_sc_w = new TH1F("FCALetato2g2pi_trg_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g2pi_trg1_Esum_sc_w = new TH1F("FCALetato2g2pi_trg1_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g2pi_trg2_Esum_sc_w = new TH1F("FCALetato2g2pi_trg2_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);
  h_FCALetato2g2pi_trg3_Esum_sc_w = new TH1F("FCALetato2g2pi_trg3_Esum_sc_w", ";E [GeV];Count [a.u.]", 120, 0., 12.);

  h_tagh = new TH2F("tagh", ";E_{#gamma} [GeV];TAGH counter;count [a.u.]", 6000, 5.0, 12.0, 500, -0.5, 499.5);
  h_tagm = new TH2F("tagm", ";E_{#gamma} [GeV];TAGM counter;t_{tagger} - t_{RF} [ns];count [a.u.]", 6000, 5.0, 12.0, 500, -0.5, 499.5);
  h_TaggerTiming_vs_eg = new TH2F("TaggerTiming_vs_eg", ";E_{#gamma} [GeV];RF-tagger;t_{tagger} - t_{RF} [ns];count [a.u.]", 500, 3.0, 12.0, 2000, -200, 200);
  h_TaggerTiming_vs_egcut = new TH2F("TaggerTiming_vs_egcut", ";E_{#gamma} [GeV];RF-tagger;t_{tagger} - t_{RF} [ns];count [a.u.]", 500, 3.0, 12.0, 2000, -200, 200);
    
  h_theta_2g = new TH1F("theta_2g", ";#theta_{#eta #rightarrow 2#gamma};count [a.u.]", 65, 0.0, 6.5);
  h_theta_6g = new TH1F("theta_6g", ";#theta_{#eta #rightarrow 3#pi^{0}};count [a.u.]", 65, 0.0, 6.5);
  h_theta_2g2pi = new TH1F("theta_2g2pi", ";#theta_{#eta #rightarrow 2#gamma#pi^{+}#pi^{-}};count [a.u.]", 65, 0.0, 6.5);
    
  h_Primakoff_2g = new TH2F("Primakoff_2g", ";m_{#gamma#gamma} / m _{#eta};(E_{#gamma}^{1} + E_{#gamma}^{2}) / E_{#gamma}^{beam};count [a.u.]", 500, 0.7, 2.0, 500, 0.3, 1.4);
  h_Primakoff_2g_r = new TH2F("Primakoff_2g_r", ";m_{#gamma#gamma} / m _{#eta};(E_{#gamma}^{1} + E_{#gamma}^{2}) / E_{#gamma}^{beam};count [a.u.]", 500, 0.7, 2.0, 500, 0.3, 1.4);
  h_Primakoff_6g = new TH2F("Primakoff_6g", 
			    ";m_{3#pi^{0}} / m _{#eta};(E_{#gamma}^{1} + E_{#gamma}^{2} + E_{#gamma}^{3} + E_{#gamma}^{4} + E_{#gamma}^{5} + E_{#gamma}^{6}) / E_{#gamma}^{beam};count [a.u.]", 
			    500, 0.7, 2.0, 500, 0.3, 1.4);
  h_Primakoff_6g_r = new TH2F("Primakoff_6g_r", 
			      ";m_{3#pi^{0}} / m _{#eta};(E_{#gamma}^{1} + E_{#gamma}^{2} + E_{#gamma}^{3} + E_{#gamma}^{4} + E_{#gamma}^{5} + E_{#gamma}^{6}) / E_{#gamma}^{beam};count [a.u.]", 
			      500, 0.7, 2.0, 500, 0.3, 1.4);
  h_Primakoff_2g2pi = new TH2F("Primakoff_2g2pi", 
			       ";m_{#pi^{0}#pi^{+}#pi^{-}} / m _{#eta};(E_{#gamma}^{1} + E_{#gamma}^{2} + E_{#pi^{+}} + E_{#pi^{-}}) / E_{#gamma}^{beam};count [a.u.]", 
			       500, 0.7, 2.0, 500, 0.3, 1.4);
  h_Primakoff_2g2pi_r = new TH2F("Primakoff_2g2pi_r", 
				 ";m_{#pi^{0}#pi^{+}#pi^{-}} / m _{#eta};(E_{#gamma}^{1} + E_{#gamma}^{2} + E_{#pi^{+}} + E_{#pi^{-}}) / E_{#gamma}^{beam};count [a.u.]", 
				 500, 0.7, 2.0, 500, 0.3, 1.4);
  
  /*
  h_trigbit->Sumw2();
  h_trig_bit->Sumw2();  
  h_trig1->Sumw2();  
  h_trig2->Sumw2();  
  h_fptrig1->Sumw2();  
  h_fptrig2->Sumw2();
  */
  h_mc_eb->Sumw2();
  h_z->Sumw2();  
  h_r->Sumw2();
  h_FCAL_trg_Esum->Sumw2();  
  h_FCAL_trg1_Esum->Sumw2();  
  h_FCAL_trg2_Esum->Sumw2();  
  h_FCAL_trg3_Esum->Sumw2();
  h_bcal_e_v_t->Sumw2();
  h_fcal_rf->Sumw2();  
  h_ccal_rf->Sumw2();
  h_fcal_xy->Sumw2();  
  h_fcal_rc->Sumw2();  
  h_fcal_tof_dx->Sumw2();
  h_Esum_bcal->Sumw2();  
  h_Esum_fcal->Sumw2();  
  h_Esum_ccal->Sumw2(); 
  h_Esum_fcal_trg1->Sumw2();  
  h_Esum_ccal_trg1->Sumw2();
  h_Esum_fcal_trg2->Sumw2();  
  h_Esum_ccal_trg2->Sumw2();
  h_Esum_fcal_trg3->Sumw2();  
  h_Esum_ccal_trg3->Sumw2();
  h_mgg_all->Sumw2();  
  h_mgg_fcal->Sumw2();  
  h_mgg_fcaltof->Sumw2();
  h_mgg->Sumw2();  
  h_mggpipi->Sumw2();
  h_mpipipi->Sumw2();  
  h_mpipipi_trg2->Sumw2();  
  h_mpipipi_trg3->Sumw2();
  h_mgg_all15->Sumw2();
  h_m6g_all->Sumw2();  
  h_m3pi0_all->Sumw2();
  h_m6g_pi0->Sumw2();  
  h_m3pi0_pi0->Sumw2();
  h_m6g_trg2->Sumw2(); 
  h_m3pi0_trg2->Sumw2();
  h_m6g_trg3->Sumw2(); 
  h_m3pi0_trg3->Sumw2();
  h_m2g_sc->Sumw2(); 
  h_m2g_sc_trg2->Sumw2();  
  h_m2g_sc_trg3->Sumw2();  
  h_m2g_sc_w->Sumw2();  
  h_m2g_sc_w_trg2->Sumw2();  
  h_m2g_sc_w_trg3->Sumw2();
  h_m6g_sc->Sumw2(); 
  h_m6g_sc_trg2->Sumw2();  
  h_m6g_sc_trg3->Sumw2();  
  h_m6g_sc_w->Sumw2();  
  h_m6g_sc_w_trg2->Sumw2();  
  h_m6g_sc_w_trg3->Sumw2(); 
  h_m2g2pi_sc->Sumw2();  
  h_m2g2pi_sc_trg2->Sumw2();  
  h_m2g2pi_sc_trg3->Sumw2();  
  h_m2g2pi_sc_w->Sumw2();  
  h_m2g2pi_sc_w_trg2->Sumw2(); 
  h_m2g2pi_sc_w_trg3->Sumw2();
  h_FCAL2g_trg_Esum_sc->Sumw2();  
  h_FCAL2g_trg1_Esum_sc->Sumw2();  
  h_FCAL2g_trg2_Esum_sc->Sumw2();  
  h_FCAL2g_trg3_Esum_sc->Sumw2(); 
  h_FCAL2g_trg_Esum_sc_w->Sumw2();  
  h_FCAL2g_trg1_Esum_sc_w->Sumw2();  
  h_FCAL2g_trg2_Esum_sc_w->Sumw2();  
  h_FCAL2g_trg3_Esum_sc_w->Sumw2();
  h_FCAL3pi0_trg_Esum_sc->Sumw2();  
  h_FCAL3pi0_trg1_Esum_sc->Sumw2();  
  h_FCAL3pi0_trg2_Esum_sc->Sumw2();  
  h_FCAL3pi0_trg3_Esum_sc->Sumw2(); 
  h_FCAL3pi0_trg_Esum_sc_w->Sumw2();  
  h_FCAL3pi0_trg1_Esum_sc_w->Sumw2();  
  h_FCAL3pi0_trg2_Esum_sc_w->Sumw2();  
  h_FCAL3pi0_trg3_Esum_sc_w->Sumw2();
  h_FCAL2g2pi_trg_Esum_sc->Sumw2();  
  h_FCAL2g2pi_trg1_Esum_sc->Sumw2();  
  h_FCAL2g2pi_trg2_Esum_sc->Sumw2();  
  h_FCAL2g2pi_trg3_Esum_sc->Sumw2(); 
  h_FCAL2g2pi_trg_Esum_sc_w->Sumw2();  
  h_FCAL2g2pi_trg1_Esum_sc_w->Sumw2();  
  h_FCAL2g2pi_trg2_Esum_sc_w->Sumw2();  
  h_FCAL2g2pi_trg3_Esum_sc_w->Sumw2();
  h_FCALetato2g_trg_Esum_sc->Sumw2();  
  h_FCALetato2g_trg1_Esum_sc->Sumw2();  
  h_FCALetato2g_trg2_Esum_sc->Sumw2();  
  h_FCALetato2g_trg3_Esum_sc->Sumw2(); 
  h_FCALetato2g_trg_Esum_sc_w->Sumw2();  
  h_FCALetato2g_trg1_Esum_sc_w->Sumw2();  
  h_FCALetato2g_trg2_Esum_sc_w->Sumw2();  
  h_FCALetato2g_trg3_Esum_sc_w->Sumw2();
  h_FCALetato3pi0_trg_Esum_sc->Sumw2();  
  h_FCALetato3pi0_trg1_Esum_sc->Sumw2();  
  h_FCALetato3pi0_trg2_Esum_sc->Sumw2();  
  h_FCALetato3pi0_trg3_Esum_sc->Sumw2(); 
  h_FCALetato3pi0_trg_Esum_sc_w->Sumw2(); 
  h_FCALetato3pi0_trg1_Esum_sc_w->Sumw2();  
  h_FCALetato3pi0_trg2_Esum_sc_w->Sumw2();  
  h_FCALetato3pi0_trg3_Esum_sc_w->Sumw2();
  h_FCALetato2g2pi_trg_Esum_sc->Sumw2();  
  h_FCALetato2g2pi_trg1_Esum_sc->Sumw2();  
  h_FCALetato2g2pi_trg2_Esum_sc->Sumw2();  
  h_FCALetato2g2pi_trg3_Esum_sc->Sumw2(); 
  h_FCALetato2g2pi_trg_Esum_sc_w->Sumw2();  
  h_FCALetato2g2pi_trg1_Esum_sc_w->Sumw2();  
  h_FCALetato2g2pi_trg2_Esum_sc_w->Sumw2();  
  h_FCALetato2g2pi_trg3_Esum_sc_w->Sumw2();
  h_theta_2g->Sumw2();  h_theta_6g->Sumw2();  
  h_theta_2g2pi->Sumw2();
  h_Primakoff_2g->Sumw2();  
  h_Primakoff_2g_r->Sumw2();
  h_Primakoff_6g->Sumw2();  
  h_Primakoff_6g_r->Sumw2();
  h_Primakoff_2g2pi->Sumw2();  
  h_Primakoff_2g2pi_r->Sumw2();
  //h_tagh->Sumw2();  
  //h_tagm->Sumw2();
  //h_TaggerTiming_vs_eg->Sumw2();  
  //h_TaggerTiming_vs_egcut->Sumw2();
  
  main->cd();

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_myanalyzer::brun(JEventLoop *eventLoop, int32_t runnumber)
{

  DGeometry*   dgeom = NULL;
  DApplication* dapp = dynamic_cast< DApplication* >( eventLoop->GetJApplication() );
  if( dapp )   dgeom = dapp->GetDGeometry( runnumber );
  
  if( dgeom ){
    dgeom->GetTargetZ( m_beamZ );
    dgeom->GetFCALPosition( m_fcalX, m_fcalY, m_fcalZ );
    dgeom->GetCCALPosition( m_ccalX, m_ccalY, m_ccalZ );
  } else{
    cerr << "No geometry accessbile to compton_analysis plugin." << endl;
    return RESOURCE_UNAVAILABLE;
  }
  
  jana::JCalibration *jcalib = japp->GetJCalibration(runnumber);
  std::map<string, float> beam_spot;
  jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  m_beamX  =  beam_spot.at("x");
  m_beamY  =  beam_spot.at("y");
  
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_myanalyzer::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
{



  Bool_t good_eta_to_2g = false;
  Bool_t good_eta_to_3pi0 = false;
  Bool_t good_eta_to_2g2pi = false;
  
  DLorentzVector TargetP4(0, 0, 0, massHe4);
  vector<const DMCThrown*> locDMCThrown;	
  eventLoop->Get(locDMCThrown);  
  vector<const DL1Trigger *> locL1Triggers;
  eventLoop->Get(locL1Triggers);
  vector< const DFCALGeometry* > fcalGeomVec;
  eventLoop->Get( fcalGeomVec );

  
  //-----   Data Objects   -----//
  vector< const DBeamPhoton *> locBeamPhotons;
  vector<const DVertex *> locVerteces;
  vector<const DNeutralParticle *> locNeutralParticles;
  vector<const DTrackTimeBased *> locTrackTimeBased;
  vector<const DChargedTrack *> locChargedTracks;
  vector<const DFCALHit *> locFCALHits;
  
  eventLoop->Get(locFCALHits);
  eventLoop->Get(locChargedTracks);
  eventLoop->Get(locBeamPhotons);
  eventLoop->Get(locVerteces);
  eventLoop->Get(locNeutralParticles);
  eventLoop->Get(locTrackTimeBased);
  const DDetectorMatches *detMatches = nullptr;
  eventLoop->GetSingle(detMatches);
    

  
  //https://logbooks.jlab.org/entry/4039521
  /*
    GTP Trigger bits:
    ----------------------
    
    Bit 0: CCAL & FCAL (prescaled)
    Bit 1: FCAL (threshold about 3.5 GeV)
    Bit 2: FCAL (threshold 0.5 GeV)
    Bit 3: PS
    Bit 10: CCAL
  */
  
  
  
  //uint32_t locL1Trigger_fp = locL1Triggers.empty() ? 0.0 : locL1Triggers[0]->fp_trig_mask;
  //uint32_t locL1Trigger = locL1Triggers.empty() ? 0.0 : locL1Triggers[0]->trig_mask;
  
  japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
  int trig_bit[33];
  if (locL1Triggers.size() > 0) {
    for (unsigned int bit = 0; bit < 32; bit ++) {
      trig_bit[bit + 1] = (locL1Triggers[0]->trig_mask & (1 << bit)) ? 1 : 0;
      if(trig_bit[bit + 1] == 1) h_trig_bit->Fill(Float_t(bit+1));
    }
  }
  //-----   Check Trigger   -----//
  uint32_t trigmask;
  uint32_t fp_trigmask;
  if (locDMCThrown.size() == 0) {
    const DL1Trigger *trig = NULL;
    try {
      eventLoop->GetSingle(trig);
    } catch (...) {
    }
    if (trig == NULL) { 
      japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
      return NOERROR;
    }
        
    trigmask = trig->trig_mask;	
    fp_trigmask = trig->fp_trig_mask;
    for (int ibit = 0; ibit < 33; ibit++) {
      if(trigmask & (1 << ibit)) h_trig1->Fill(ibit);
      if(fp_trigmask & (1 << ibit)) h_fptrig1->Fill(ibit);
    }
    if( trigmask==8 ) {
      japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
      return NOERROR;
    }
    if( fp_trigmask ) {
      japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
      return NOERROR;
    }
  } else {
    trigmask = 1;
    fp_trigmask = 1;
  }
  
  h_trig2->Fill(trigmask);
  h_fptrig2->Fill(fp_trigmask);
  japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
  //-----   RF Bunch   -----//
  
  const DEventRFBunch *locRFBunch = NULL;
  try { 
    eventLoop->GetSingle( locRFBunch, "CalorimeterOnly" );
  } catch (...) { return NOERROR; }
  double locRFTime = locRFBunch->dTime;
  if( locRFBunch->dNumParticleVotes < 2 ) return NOERROR;
  
  
  //-----   Geometry   -----//
    
  if( fcalGeomVec.size() != 1 ) {
    cerr << "No FCAL geometry accessbile." << endl;
    return RESOURCE_UNAVAILABLE;
  }
  
  const DFCALGeometry *fcalGeom = fcalGeomVec[0];
  
  //Retrieve vertex info
  DVector3 vertex;
  vertex.SetXYZ( m_beamX, m_beamY, m_beamZ );
  
  double kinfitVertexX = m_beamX;
  double kinfitVertexY = m_beamY;
  double kinfitVertexZ = m_beamZ;
  double kinfitR = 0;
  japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
  for (unsigned int i = 0 ; i < locVerteces.size(); i++) {
    kinfitVertexX = locVerteces[i]->dSpacetimeVertex.X();
    kinfitVertexY = locVerteces[i]->dSpacetimeVertex.Y();
    kinfitVertexZ = locVerteces[i]->dSpacetimeVertex.Z();
    kinfitR = sqrt(pow(kinfitVertexX, 2) + pow(kinfitVertexY, 2));
    h_z->Fill(kinfitVertexZ);
    h_r->Fill(kinfitR);
  }

  Bool_t target_vtx_z = (50.0 < kinfitVertexZ && kinfitVertexZ < 80);
  Bool_t target_vtx_r = sqrt(pow(kinfitVertexX, 2) + pow(kinfitVertexY, 2)) < 1;

  //For MC simu.
  double mc_eb = 0;
  if (locDMCThrown.size() > 0) {
    const DMCReaction* locMCReactions = NULL;
    eventLoop->GetSingle(locMCReactions);
    mc_eb = locMCReactions->beam.energy();
    h_mc_eb->Fill(mc_eb);
    for (unsigned int bit = 0; bit < 32; bit ++) {
      trig_bit[bit + 1] = 1;
    }
  }

  //Compute the FCAL energy sum for cell with an energy deposited above 150MeV 
  double BCAL_Esum = 0;
  double FCAL_Esum = 0;
  double CCAL_Esum = 0;
  int n_locBCALShowers = 0;
  int n_locFCALShowers = 0;
  int n_locFCALTOFShowers = 0;
  int n_locCCALShowers = 0;
  double FCAL_trg_Esum = 0;

  for (vector<const DFCALHit*>::const_iterator hit  = locFCALHits.begin(); hit != locFCALHits.end(); hit++ ) {
    if ((**hit).E > 0.150)
      FCAL_trg_Esum += (**hit).E;
  }

  if (FCAL_trg_Esum > 0) {
    h_FCAL_trg_Esum->Fill(FCAL_trg_Esum);
    if (trig_bit[1] == 1)
      h_FCAL_trg1_Esum->Fill(FCAL_trg_Esum);
    if (trig_bit[2] == 1)
      h_FCAL_trg2_Esum->Fill(FCAL_trg_Esum);
    if (trig_bit[3] == 1)
      h_FCAL_trg3_Esum->Fill(FCAL_trg_Esum);
  }
  //Loop over neutral particle list, showers matching a track a removed
  vector<const DNeutralParticleHypothesis*> PhotonsList;
  Bool_t InnerFCAL_ring = false;

  for (unsigned int i = 0; i < locNeutralParticles.size(); i++){
    
    const DNeutralParticleHypothesis * photon = locNeutralParticles[i]->Get_Hypothesis(Gamma);
    if (photon == nullptr) continue;
    
    const DNeutralShower * shower = photon->Get_NeutralShower();
    
    if (shower->dDetectorSystem == SYS_BCAL) { 
      
      DBCALShower *bcal_shower = (DBCALShower *) shower->dBCALFCALShower;
      double e = bcal_shower->E;
      double x = bcal_shower->x - vertex.X();
      double y = bcal_shower->y - vertex.Y();
      double z = bcal_shower->z - vertex.Z();
      DVector3 position(x, y, z);
      double r = position.Mag();
      double t = bcal_shower->t - (r / TMath::C() * 1e7);
      double px = e * sin(position.Theta()) * cos(position.Phi());
      double py = e * sin(position.Theta()) * sin(position.Phi());
      double pz = e * cos(position.Theta());
      DLorentzVector PhotonVec(px, py, pz, e);
      double diff_t = t - locRFTime;
      h_bcal_e_v_t->Fill(diff_t, e);
      if (fabs(diff_t) < BCAL_RF_CUT) {
	if (e > 0.25) PhotonsList.push_back(photon);
	n_locBCALShowers ++;
	BCAL_Esum += e;
      }
    }
    
    if (shower->dDetectorSystem == SYS_FCAL) { 
      
      DFCALShower *fcal_shower = (DFCALShower *) shower->dBCALFCALShower;
      double e = fcal_shower->getEnergy();
      DVector3 position = fcal_shower->getPosition_log() - vertex;
      //DVector3 position = fcal_shower->getPosition() - vertex;
      double r = position.Mag();
      double t = fcal_shower->getTime() - (r / TMath::C() * 1e7);
      double px = e * sin(position.Theta()) * cos(position.Phi());
      double py = e * sin(position.Theta()) * sin(position.Phi());
      double pz = e * cos(position.Theta());
      DLorentzVector PhotonVec(px, py, pz, e);
      double diff_t = t - locRFTime;
      double face_x = vertex.X() + (position.X() * (m_fcalZ - vertex.Z())/position.Z());
      double face_y = vertex.Y() + (position.Y() * (m_fcalZ - vertex.Z())/position.Z());
      int row = fcalGeom->row(static_cast<float>(face_y));
      int col = fcalGeom->column(static_cast<float>(face_x));
      //int layer = fcalLayer(row, col);
      const DFCALCluster *fcalCluster;
      fcal_shower->GetSingle(fcalCluster);
      //int channel = fcalCluster->getChannelEmax();
      //double emax = fcalCluster->getEmax();
      float TOF_FCAL_x_min = shower->dTOF_FCAL_x_min;
      float TOF_FCAL_y_min = shower->dTOF_FCAL_y_min;

      //bool in_insert = fcalGeom->inInsert(channel);
      double radius = sqrt(pow(position.X(), 2) + pow(position.Y(), 2));
      
      h_fcal_rf->Fill(diff_t);
      h_fcal_xy->Fill(face_x, face_y);
      h_fcal_rc->Fill(row, col);
      h_fcal_tof_dx->Fill(TOF_FCAL_x_min, TOF_FCAL_y_min);
      
      if (fabs(diff_t) < FCAL_RF_CUT) {
	if (radius < 10) InnerFCAL_ring = true;
	if (e > 0.25) PhotonsList.push_back(photon);
	n_locFCALShowers ++;
	if (TOF_FCAL_x_min > 6 && TOF_FCAL_y_min > 6) {
	  n_locFCALTOFShowers ++;
	}
	FCAL_Esum += e;
      }
    }

    
    if (shower->dDetectorSystem == SYS_CCAL) {
      
      DCCALShower *ccal_shower = (DCCALShower *) shower->dBCALFCALShower;
      double e = ccal_shower->E;
      double x = ccal_shower->x1 - vertex.X();
      double y = ccal_shower->y1 - vertex.Y();
      double z = ccal_shower->z - vertex.Z();
      DVector3 position(x, y, z);
      double r = position.Mag();
      double t = ccal_shower->time - (r / TMath::C() * 1e7);
      double px = e * sin(position.Theta()) * cos(position.Phi());
      double py = e * sin(position.Theta()) * sin(position.Phi());
      double pz = e * cos(position.Theta());
      DLorentzVector PhotonVec(px, py, pz, e);
      double diff_t = t - locRFTime;
      //double phi_ccal = PhotonVec.Phi();
      //double theta_ccal = PhotonVec.Theta();
      //double delta_phi_min = 1000.;
      h_ccal_rf->Fill(diff_t);
      if (fabs(diff_t) < CCAL_RF_CUT) {
	//ShowersList.push_back(PhotonVec);
	n_locCCALShowers++;
	CCAL_Esum += e;
      }
    }
  }
  
  //Checking showers energy sum
  if (n_locBCALShowers > 0)
    h_Esum_bcal->Fill(BCAL_Esum);
  if (n_locFCALShowers > 0) {
    h_Esum_fcal->Fill(FCAL_Esum);
    if (trig_bit[1] == 1) {
      h_Esum_fcal_trg1->Fill(FCAL_Esum);
    }
    if (trig_bit[2] == 1) {
      h_Esum_fcal_trg2->Fill(FCAL_Esum);
    }
    if (trig_bit[3] == 1) {
      h_Esum_fcal_trg3->Fill(FCAL_Esum);
    }
  }
  if (n_locCCALShowers > 0) {
    h_Esum_ccal->Fill(CCAL_Esum);
    if (trig_bit[1] == 1) {
      h_Esum_ccal_trg1->Fill(CCAL_Esum);
    }
    if (trig_bit[2] == 1) {
      h_Esum_ccal_trg2->Fill(CCAL_Esum);
    }
    if (trig_bit[3] == 1) {
      h_Esum_ccal_trg3->Fill(CCAL_Esum);
    }
  }
  japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
  //Retrieve tracks info and assign it a pi hypo
  vector <const DTrackTimeBased *> pimsList;
  vector <const DTrackTimeBased *> pipsList;
  FillParticleVectors(locChargedTracks, pimsList, pipsList);
  DLorentzVector EtaP4;
  DLorentzVector gP4[6];
  DLorentzVector pipP4;
  DLorentzVector pimP4;
  japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
  //Looking at event with 2 unmatched showers
  if (PhotonsList.size() == 2) {
    
    gP4[0] = PhotonsList[0]->lorentzMomentum();
    gP4[1] = PhotonsList[1]->lorentzMomentum();
    DLorentzVector ggP4 = gP4[0] + gP4[1];
    good_eta_to_2g = true;
    EtaP4 = ggP4;

    h_mgg_all->Fill(ggP4.M());
    
    if (n_locFCALShowers == 2)
      h_mgg_fcal->Fill(ggP4.M());
    
    if (n_locFCALTOFShowers == 2)
      h_mgg_fcaltof->Fill(ggP4.M());
    
    Bool_t GoodPi0 = (0.11 < ggP4.M() && ggP4.M() < 0.16);

    //Looking at event with 2 unmatched showers and a pair of opposite charged tracks
    if (pimsList.size() == 1 && pipsList.size() == 1) {
      
      pipP4 = pimsList[0]->lorentzMomentum();
      pimP4 = pipsList[0]->lorentzMomentum();
      DLorentzVector ggpipiP4 = ggP4 + pipP4 + pimP4;
      
      h_mgg->Fill(ggP4.M());
      h_mggpipi->Fill(ggpipiP4.M());
      
      if (GoodPi0) {
	good_eta_to_2g2pi = true;
	EtaP4 = ggpipiP4;
	h_mpipipi->Fill(ggpipiP4.M());
	if (trig_bit[2] == 1) {
	  h_mpipipi_trg2->Fill(ggpipiP4.M());
	}
	if (trig_bit[3] == 1) {
	  h_mpipipi_trg3->Fill(ggpipiP4.M());
	}
      }
    }
  }
  //Looking at event with 6 unmatched showers
  if (PhotonsList.size() == 6) {
    
    Double_t bestRawChi2Eta = 1.0e30;
    Double_t bestRawChi2EtaPrim = 1.0e30;
    vector <int> Index6gList;Index6gList.clear();
    vector <int> Indexprim6gList;Indexprim6gList.clear();
    
    //Find the best combo
    Combined6g(PhotonsList,
	       bestRawChi2Eta,
	       bestRawChi2EtaPrim,
	       Index6gList,
	       Indexprim6gList);

    //Select the best combo
    if (Index6gList.size() == 6) {
      for (int i = 0; i < 6; i ++) {
	gP4[i] = PhotonsList[Index6gList[i]]->lorentzMomentum();
      }
      
      double m0 = (PhotonsList[Index6gList[0]]->lorentzMomentum() + PhotonsList[Index6gList[1]]->lorentzMomentum()).M();
      double m1 = (PhotonsList[Index6gList[2]]->lorentzMomentum() + PhotonsList[Index6gList[3]]->lorentzMomentum()).M();
      double m2 = (PhotonsList[Index6gList[4]]->lorentzMomentum() + PhotonsList[Index6gList[5]]->lorentzMomentum()).M();
      
      DLorentzVector ggggggP4 = 
	PhotonsList[Index6gList[0]]->lorentzMomentum() + PhotonsList[Index6gList[1]]->lorentzMomentum() + 
	PhotonsList[Index6gList[2]]->lorentzMomentum() + PhotonsList[Index6gList[3]]->lorentzMomentum() + 
	PhotonsList[Index6gList[4]]->lorentzMomentum() + PhotonsList[Index6gList[5]]->lorentzMomentum();
      
      DLorentzVector pi0pi0pi0P4 = 
	m_pi0 / m0 * (PhotonsList[Index6gList[0]]->lorentzMomentum() + PhotonsList[Index6gList[1]]->lorentzMomentum()) +
	m_pi0 / m1 * (PhotonsList[Index6gList[2]]->lorentzMomentum() + PhotonsList[Index6gList[3]]->lorentzMomentum()) +
	m_pi0 / m2 * (PhotonsList[Index6gList[4]]->lorentzMomentum() + PhotonsList[Index6gList[5]]->lorentzMomentum());
      
      int pi0_nb = 0;
      for (int i = 0; i < 3; i ++) {
	DLorentzVector g1P4 = PhotonsList[Index6gList[2 * i]]->lorentzMomentum();
	DLorentzVector g2P4 = PhotonsList[Index6gList[2 * i + 1]]->lorentzMomentum();
	DLorentzVector ggP4 = g1P4 + g2P4;
	h_mgg_all15->Fill(ggP4.M());
	Bool_t GoodPi0 = (0.11 < ggP4.M() && ggP4.M() < 0.16);
	if (GoodPi0) pi0_nb ++;
      }
      
      h_m6g_all->Fill(ggggggP4.M());
      h_m3pi0_all->Fill(pi0pi0pi0P4.M());
      
      if (pi0_nb == 3) {
	good_eta_to_3pi0 = true;
	EtaP4 = pi0pi0pi0P4;
	h_m6g_pi0->Fill(ggggggP4.M());
	h_m3pi0_pi0->Fill(pi0pi0pi0P4.M());
	if (trig_bit[2] == 1) {
	  h_m6g_trg2->Fill(ggggggP4.M());
	  h_m3pi0_trg2->Fill(pi0pi0pi0P4.M());
	}
	if (trig_bit[3] == 1) {
	  h_m6g_trg3->Fill(ggggggP4.M());
	  h_m3pi0_trg3->Fill(pi0pi0pi0P4.M());
	}
      }
    }
  }
  //Basic selection criteria
  Bool_t Prim2g = good_eta_to_2g * (locChargedTracks.size() == 0 && n_locBCALShowers == 0 && n_locFCALTOFShowers == 2);
  Bool_t Prim3pi0 = good_eta_to_3pi0 * (locChargedTracks.size() == 0);
  Bool_t Prim2g2pi = good_eta_to_2g2pi * target_vtx_z * target_vtx_r * (locChargedTracks.size() == 2);
  
  //Loop over beam-photon if basic selection criteria is fullfilled
  if ((Prim2g || Prim3pi0 || Prim2g2pi) 
      && 
      (trig_bit[2] == 1 || trig_bit[3] == 1)
      &&
      !InnerFCAL_ring) {
    
    if (Prim2g) {
      h_m2g_sc->Fill(EtaP4.M());
      if (trig_bit[2] == 1)
	h_m2g_sc_trg2->Fill(EtaP4.M());
      if (trig_bit[3] == 1)
	h_m2g_sc_trg3->Fill(EtaP4.M());
    }
    if (Prim3pi0) {
      h_m6g_sc->Fill(EtaP4.M());
      if (trig_bit[2] == 1)
	h_m6g_sc_trg2->Fill(EtaP4.M());
      if (trig_bit[3] == 1)
	h_m6g_sc_trg3->Fill(EtaP4.M());
    }
    if (Prim2g2pi) {
      h_m2g2pi_sc->Fill(EtaP4.M());
      if (trig_bit[2] == 1)
	h_m2g2pi_sc_trg2->Fill(EtaP4.M());
      if (trig_bit[3] == 1)
	h_m2g2pi_sc_trg3->Fill(EtaP4.M());
    }

    if (FCAL_trg_Esum > 0) {
      if (Prim2g) {
	h_FCAL2g_trg_Esum_sc->Fill(FCAL_trg_Esum);
	if (trig_bit[1] == 1)
	  h_FCAL2g_trg1_Esum_sc->Fill(FCAL_trg_Esum);
	if (trig_bit[2] == 1)
	  h_FCAL2g_trg2_Esum_sc->Fill(FCAL_trg_Esum);
	if (trig_bit[3] == 1)
	  h_FCAL2g_trg3_Esum_sc->Fill(FCAL_trg_Esum);
	if (0.5 < EtaP4.M() && EtaP4.M() < 0.6) {
	  h_FCALetato2g_trg_Esum_sc->Fill(FCAL_trg_Esum);
	  if (trig_bit[1] == 1)
	    h_FCALetato2g_trg1_Esum_sc->Fill(FCAL_trg_Esum);
	  if (trig_bit[2] == 1)
	    h_FCALetato2g_trg2_Esum_sc->Fill(FCAL_trg_Esum);
	  if (trig_bit[3] == 1)
	    h_FCALetato2g_trg3_Esum_sc->Fill(FCAL_trg_Esum);
	}
      }
      if (Prim3pi0) {
	h_FCAL3pi0_trg_Esum_sc->Fill(FCAL_trg_Esum);
	if (trig_bit[1] == 1)
	  h_FCAL3pi0_trg1_Esum_sc->Fill(FCAL_trg_Esum);
	if (trig_bit[2] == 1)
	  h_FCAL3pi0_trg2_Esum_sc->Fill(FCAL_trg_Esum);
	if (trig_bit[3] == 1)
	  h_FCAL3pi0_trg3_Esum_sc->Fill(FCAL_trg_Esum);
	if (0.5 < EtaP4.M() && EtaP4.M() < 0.6) {
	  h_FCALetato3pi0_trg_Esum_sc->Fill(FCAL_trg_Esum);
	  if (trig_bit[1] == 1)
	    h_FCALetato3pi0_trg1_Esum_sc->Fill(FCAL_trg_Esum);
	  if (trig_bit[2] == 1)
	    h_FCALetato3pi0_trg2_Esum_sc->Fill(FCAL_trg_Esum);
	  if (trig_bit[3] == 1)
	    h_FCALetato3pi0_trg3_Esum_sc->Fill(FCAL_trg_Esum);
	}
      }
      if (Prim2g2pi) {
	h_FCAL2g2pi_trg_Esum_sc->Fill(FCAL_trg_Esum);
	if (trig_bit[1] == 1)
	  h_FCAL2g2pi_trg1_Esum_sc->Fill(FCAL_trg_Esum);
	if (trig_bit[2] == 1)
	  h_FCAL2g2pi_trg2_Esum_sc->Fill(FCAL_trg_Esum);
	if (trig_bit[3] == 1)
	  h_FCAL2g2pi_trg3_Esum_sc->Fill(FCAL_trg_Esum);
	if (0.5 < EtaP4.M() && EtaP4.M() < 0.6) {
	  h_FCALetato2g2pi_trg_Esum_sc->Fill(FCAL_trg_Esum);
	  if (trig_bit[1] == 1)
	    h_FCALetato2g2pi_trg1_Esum_sc->Fill(FCAL_trg_Esum);
	  if (trig_bit[2] == 1)
	    h_FCALetato2g2pi_trg2_Esum_sc->Fill(FCAL_trg_Esum);
	  if (trig_bit[3] == 1)
	    h_FCALetato2g2pi_trg3_Esum_sc->Fill(FCAL_trg_Esum);
	}
      }
    }
    
    //Loop over beam-photon
    for (unsigned int i = 0; i < locBeamPhotons.size(); i ++) {
      
      const DBeamPhoton * ebeam = locBeamPhotons[i]; 
      double eb = ebeam->lorentzMomentum().E();	
      DetectorSystem_t sys = ebeam->dSystem;
      int counter = ebeam->dCounter;
      //int tagm_ctr = -1;
      //int tagh_ctr = -1;
      if (sys == SYS_TAGH) {
	h_tagh->Fill(eb, counter);
	//tagh_ctr = counter;
      } else if (sys == SYS_TAGM) {
	h_tagm->Fill(eb, counter);
	//tagm_ctr = counter;
      }
      if (eb < 8.0) continue;
      double tb = ebeam->time();
      double zb = ebeam->position().Z();
      DLorentzVector PhotonBeamP4(0, 0, eb, eb);
      DLorentzVector ISP4 = TargetP4 + PhotonBeamP4;
      double locDeltaTRF = tb - (locRFTime + (zb - m_beamZ) / 29.9792458);
      h_TaggerTiming_vs_eg->Fill(eb, locDeltaTRF);
      double weight = 0;
      if (fabs(locDeltaTRF) <= 2.004) {
	weight = 1;
      } else if ( ( -(2.004 + 6.0 * 4.008) <= locDeltaTRF && locDeltaTRF <= -(2.004 + 4.008) ) || 
		  ( (2.004 + 4.008) <= locDeltaTRF && locDeltaTRF <= (2.004 + 6.0 * 4.008) ) ) {
	weight = -0.1;
      } else {
	continue;
      }
      h_TaggerTiming_vs_egcut->Fill(eb, locDeltaTRF);
      
      //double DE = eb - EtaP4.E();
      double M = EtaP4.M();
      //double rM = M * cos(45.0 * TMath::RadToDeg()) - DE * sin(45.0 * TMath::RadToDeg());
      double RatioE = EtaP4.E() / eb;
      double RatiorE = (m_eta / M * EtaP4).E() / eb;
      double RatioM = EtaP4.M() / m_eta; 
      //double RatiorE = RatioM * cos(45.0 * TMath::RadToDeg()) - RatioE * sin(45.0 * TMath::RadToDeg());; 
      double theta_eta = EtaP4.Theta() * TMath::RadToDeg();
      
      if (Prim2g) {
	h_Primakoff_2g->Fill(RatioM, RatioE, weight);
	h_Primakoff_2g_r->Fill(RatioM, RatiorE, weight);
      }
      if (Prim3pi0) {
	h_Primakoff_6g->Fill(RatioM, RatioE, weight);
	h_Primakoff_6g_r->Fill(RatioM, RatiorE, weight);
      }
      if (Prim2g2pi) {
	h_Primakoff_2g2pi->Fill(RatioM, RatioE, weight);
	h_Primakoff_2g2pi_r->Fill(RatioM, RatiorE, weight);
      }
      Bool_t elasticity = (0.9 < RatioE && RatioE < 1.1);
      Bool_t good_eta = (0.9 < RatioM && RatioM < 1.1);
      if (elasticity) {	
	if (Prim2g) {
	  h_m2g_sc_w->Fill(EtaP4.M(), weight);
	  if (trig_bit[2] == 1)
	    h_m2g_sc_w_trg2->Fill(EtaP4.M(), weight);
	  if (trig_bit[3] == 1)
	    h_m2g_sc_w_trg3->Fill(EtaP4.M(), weight);
	}
	if (Prim3pi0) {
	  h_m6g_sc_w->Fill(EtaP4.M(), weight);
	  if (trig_bit[2] == 1)
	    h_m6g_sc_w_trg2->Fill(EtaP4.M(), weight);
	  if (trig_bit[3] == 1)
	    h_m6g_sc_w_trg3->Fill(EtaP4.M(), weight);
	}
	if (Prim2g2pi) {
	  h_m2g2pi_sc_w->Fill(EtaP4.M(), weight);
	  if (trig_bit[2] == 1)
	    h_m2g2pi_sc_w_trg2->Fill(EtaP4.M(), weight);
	  if (trig_bit[3] == 1)
	    h_m2g2pi_sc_w_trg3->Fill(EtaP4.M(), weight);
	}
	
	if (FCAL_trg_Esum > 0) {
	  if (Prim2g) {
	    h_FCAL2g_trg_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (trig_bit[1] == 1)
	      h_FCAL2g_trg1_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (trig_bit[2] == 1)
	      h_FCAL2g_trg2_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (trig_bit[3] == 1)
	      h_FCAL2g_trg3_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (0.5 < EtaP4.M() && EtaP4.M() < 0.6) {
	      h_FCALetato2g_trg_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	      if (trig_bit[1] == 1)
		h_FCALetato2g_trg1_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	      if (trig_bit[2] == 1)
		h_FCALetato2g_trg2_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	      if (trig_bit[3] == 1)
		h_FCALetato2g_trg3_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    }
	  }
	  if (Prim3pi0) {
	    h_FCAL3pi0_trg_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (trig_bit[1] == 1)
	      h_FCAL3pi0_trg1_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (trig_bit[2] == 1)
	      h_FCAL3pi0_trg2_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (trig_bit[3] == 1)
	      h_FCAL3pi0_trg3_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (0.5 < EtaP4.M() && EtaP4.M() < 0.6) {
	      h_FCALetato3pi0_trg_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	      if (trig_bit[1] == 1)
		h_FCALetato3pi0_trg1_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	      if (trig_bit[2] == 1)
		h_FCALetato3pi0_trg2_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	      if (trig_bit[3] == 1)
		h_FCALetato3pi0_trg3_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    }
	  }
	  if (Prim2g2pi) {
	    h_FCAL2g2pi_trg_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (trig_bit[1] == 1)
	      h_FCAL2g2pi_trg1_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (trig_bit[2] == 1)
	      h_FCAL2g2pi_trg2_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (trig_bit[3] == 1)
	      h_FCAL2g2pi_trg3_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    if (0.5 < EtaP4.M() && EtaP4.M() < 0.6) {
	      h_FCALetato2g2pi_trg_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	      if (trig_bit[1] == 1)
		h_FCALetato2g2pi_trg1_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	      if (trig_bit[2] == 1)
		h_FCALetato2g2pi_trg2_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	      if (trig_bit[3] == 1)
		h_FCALetato2g2pi_trg3_Esum_sc_w->Fill(FCAL_trg_Esum, weight);
	    }
	  }
	}
      }

      if (good_eta && elasticity && (0.5 < EtaP4.M() && EtaP4.M() < 0.6)) {
	if (Prim2g)
	  h_theta_2g->Fill(theta_eta, weight);

	if (Prim3pi0)
	  h_theta_6g->Fill(theta_eta, weight);
	
	if (Prim2g2pi)
	  h_theta_2g2pi->Fill(theta_eta, weight);
      }
    }
  }
  
  japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_myanalyzer::erun(void)
{
	
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_myanalyzer::fini(void)
{
	
	return NOERROR;
}

jerror_t JEventProcessor_myanalyzer::FillParticleVectors(vector<const DChargedTrack *>&locChargedTracks,
							 vector<const DTrackTimeBased *>&pims,
							 vector<const DTrackTimeBased *>&pips) {
  
  for (unsigned int j = 0; j < locChargedTracks.size(); j ++) {    
  
    const DTrackTimeBased * pion_track = NULL;
    const DChargedTrackHypothesis * hyp = NULL;
    bool got_piplus = false;
    double pion_prob = 0.;
    //double pion_dEdx = 0.;
    //double pion_bg = 0.;
    
    // Look at pions
    hyp = locChargedTracks[j]->Get_Hypothesis(PiPlus);
    if (hyp != NULL) {
      shared_ptr<const DTOFHitMatchParams> tofparms = hyp->Get_TOFHitMatchParams();
      double dt=1000.;
      pion_track = hyp->Get_TrackTimeBased();
      double prob = TMath::Prob(pion_track->chisq, pion_track->Ndof);
      if (prob > 1.e-6) {
	double p = pion_track->momentum().Mag();
	double betagamma = p / pion_track->mass();
	double dEdxSigma = 0.289 / (betagamma * betagamma) - 0.349 / betagamma + 0.321;
	double dEdx = 1e6 * pion_track->dEdx();
	double sum_chi2 = 0;
	int num_dof = 0;
	if (dEdx > 0) {
	  double bg2 = betagamma * betagamma;
	  double dEdxmean=0.061 / (bg2*bg2) + 1.15 / bg2 + 1.21;
	  double ratio = (dEdx - dEdxmean) / dEdxSigma;
	  sum_chi2 += ratio * ratio;
	  num_dof ++;
	}
	if (tofparms != NULL) {
	  dt = tofparms->dHitTime-tofparms->dFlightTime-pion_track->t0();
	  double SigmaT=0.4; //??
	  double ratio= dt / SigmaT;
	  sum_chi2 += ratio * ratio;
	  num_dof ++;
	}
	pion_prob = TMath::Prob(sum_chi2, num_dof);
	//pion_bg = betagamma;
	//pion_dEdx = dEdx;
      }
    }
    if (pion_track != NULL && pion_prob > 1e-6) {
      got_piplus=true;
      pips.push_back(pion_track);
    }
    // Next deal with negative particles
    if (got_piplus == false){
      pion_prob = 0.;
      pion_track = NULL;
      //pion_dEdx = 0.;
      //pion_bg = 0.;
      
      hyp = locChargedTracks[j]->Get_Hypothesis(PiMinus);
      if (hyp != NULL) {
	shared_ptr<const DTOFHitMatchParams> tofparms = hyp->Get_TOFHitMatchParams();
	double dt = 1000.;//,dt_bcal=1000.;
	
	pion_track = hyp->Get_TrackTimeBased();
	double prob = TMath::Prob(pion_track->chisq, pion_track->Ndof);
	if (prob > 1.e-6) {
	  double p = pion_track->momentum().Mag();
	  double betagamma = p / pion_track->mass();
	  double dEdxSigma = 0.289 / (betagamma * betagamma) - 0.349 / betagamma + 0.321;
	  double dEdx = 1e6 * pion_track->dEdx();
	  double sum_chi2 = 0;
	  int num_dof = 0;
	  if (dEdx > 0) {
	    double bg2 = betagamma * betagamma;
	    double dEdxmean = 0.061 / (bg2 * bg2) + 1.15 / bg2 + 1.21;
	    double ratio = (dEdx - dEdxmean) / dEdxSigma;
	    sum_chi2 += ratio * ratio;
	    num_dof ++;
	  }
	  if (tofparms != NULL) {
	    dt = tofparms->dHitTime - tofparms->dFlightTime-pion_track->t0();
	    double SigmaT = 0.4; //??
	    double ratio = dt / SigmaT;
	    sum_chi2 += ratio * ratio;
	    num_dof ++;
	  }
	  
	  pion_prob = TMath::Prob(sum_chi2, num_dof);
	  //pion_bg = betagamma;
	  //pion_dEdx = dEdx;
	}
      }
      
      if (pion_track!=NULL && pion_prob > 1e-6) {
	
	pims.push_back(pion_track);
      }
    }
  }
  
  return NOERROR;
}

void JEventProcessor_myanalyzer::Combined6g(vector<const DNeutralParticleHypothesis *>&EMList,
    Double_t &bestChi0Eta,
    Double_t &bestChi0EtaPrim,
    vector<int>&Index6gList,
    vector<int>&Indexprim6gList)
{	  
  bestChi0EtaPrim   = 1.0e30;
  bestChi0Eta       = 1.0e30;
  if(EMList.size() == 6) {
    for (int i_comb = 0; i_comb < (*combi6).getMaxCombi(); i_comb ++) {
      combi6->getCombi(i_comb);

      double Esum = 0.0;		  
      for (int i = 0; i < 6; i ++) {
        Esum += (EMList[combi6->combi[i]]->lorentzMomentum()).E();
      }

      double Chi2_pi0Mass[3];
      double Chi2_etaMass[3];
      vector<DLorentzVector>GG;GG.clear();
      vector<DLorentzVector>Pi0Cor;Pi0Cor.clear();
      vector<DLorentzVector>EtaCor;EtaCor.clear();

      for (int i = 0; i < 3; i ++) {
        GG.push_back(EMList[combi6->combi[2*i]]->lorentzMomentum() + EMList[combi6->combi[2*i+1]]->lorentzMomentum());
      }
      for (int i = 0; i < 3; i ++) {
        Pi0Cor.push_back( m_pi0 / GG[i].M() * GG[i] );
        EtaCor.push_back( m_eta / GG[i].M() * GG[i] );
        Chi2_pi0Mass[i] = TMath::Power((GG[i].M() - m_pi0) / 8.0e-3,2.0);
        Chi2_etaMass[i] = TMath::Power((GG[i].M() - m_eta) / 22.0e-3,2.0);
      }

      double Chi2_3pi0      = Chi2_pi0Mass[0] + Chi2_pi0Mass[1] + Chi2_pi0Mass[2]; 
      double Chi2_2pi0eta_0 = Chi2_pi0Mass[0] + Chi2_pi0Mass[1] + Chi2_etaMass[2];
      double Chi2_2pi0eta_1 = Chi2_pi0Mass[0] + Chi2_etaMass[1] + Chi2_pi0Mass[2];
      double Chi2_2pi0eta_2 = Chi2_etaMass[0] + Chi2_pi0Mass[1] + Chi2_pi0Mass[2];

      if (Esum > 500.0e-3) {
        DLorentzVector EtaVec = Pi0Cor[0] + Pi0Cor[1] + Pi0Cor[2];
        bool AnEta     = false; 
        if (GG[0].M() > 110.0e-3 && 
            GG[1].M() > 110.0e-3 && 
            GG[2].M() > 110.0e-3 
            && 
            GG[0].M() < 160.0e-3 && 
            GG[1].M() < 160.0e-3 && 
            GG[2].M() < 160.0e-3)
          AnEta = true;

        bool AnEtaPrim  = false; 
        bool AnEtaPrim1 = false; 
        bool AnEtaPrim2 = false; 
        bool AnEtaPrim3 = false; 
        if (GG[0].M() > 110.0e-3 && 
            GG[1].M() > 110.0e-3 && 
            GG[2].M() > 500.0e-3 
            && 
            GG[0].M() < 160.0e-3 && 
            GG[1].M() < 160.0e-3 && 
            GG[2].M() < 600.0e-3)
          AnEtaPrim1 = true;
        if (GG[0].M() > 110.0e-3 && 
            GG[1].M() > 500.0e-3 && 
            GG[2].M() > 110.0e-3 
            && 
            GG[0].M() < 160.0e-3 && 
            GG[1].M() < 600.0e-3 && 
            GG[2].M() < 160.0e-3)
          AnEtaPrim2 = true;
        if (GG[0].M() > 500.0e-3 && 
            GG[1].M() > 110.0e-3 && 
            GG[2].M() > 110.0e-3 
            && 
            GG[0].M() < 600.0e-3 && 
            GG[1].M() < 160.0e-3 && 
            GG[2].M() < 160.0e-3)
          AnEtaPrim3 = true;
        if (AnEtaPrim1 || AnEtaPrim2 || AnEtaPrim3)
          AnEtaPrim = true;

        if (AnEta && !AnEtaPrim && Esum > 500.0e-3) {
          if (Chi2_3pi0<bestChi0Eta) {
            bestChi0Eta = Chi2_3pi0;
            Index6gList.clear();
            Index6gList.push_back(combi6->combi[0]);
            Index6gList.push_back(combi6->combi[1]);
            Index6gList.push_back(combi6->combi[2]);
            Index6gList.push_back(combi6->combi[3]);
            Index6gList.push_back(combi6->combi[4]);
            Index6gList.push_back(combi6->combi[5]);
          }
        }

        if (!AnEta && AnEtaPrim && Esum > 900.0e-3) {
          double Esum2gg1 = EMList[combi6->combi[4]]->lorentzMomentum().E() + EMList[combi6->combi[5]]->lorentzMomentum().E();
          if (AnEtaPrim1 && Esum2gg1 > 500.0e-3) {
            if (Chi2_2pi0eta_0 < bestChi0EtaPrim) {
              bestChi0EtaPrim = Chi2_2pi0eta_0;
              Indexprim6gList.clear();
              Indexprim6gList.push_back(combi6->combi[0]);
              Indexprim6gList.push_back(combi6->combi[1]);
              Indexprim6gList.push_back(combi6->combi[2]);
              Indexprim6gList.push_back(combi6->combi[3]);
              Indexprim6gList.push_back(combi6->combi[4]);
              Indexprim6gList.push_back(combi6->combi[5]);
            }
          }
	  double Esum2gg2 = EMList[combi6->combi[2]]->lorentzMomentum().E() + EMList[combi6->combi[3]]->lorentzMomentum().E();
          if (AnEtaPrim2 && Esum2gg2 > 500.0e-3) {
	    if (Chi2_2pi0eta_1 < bestChi0EtaPrim) {
	      bestChi0EtaPrim = Chi2_2pi0eta_1;
              Indexprim6gList.clear();
              Indexprim6gList.push_back(combi6->combi[0]);
              Indexprim6gList.push_back(combi6->combi[1]);
              Indexprim6gList.push_back(combi6->combi[4]);
              Indexprim6gList.push_back(combi6->combi[5]);
              Indexprim6gList.push_back(combi6->combi[2]);
              Indexprim6gList.push_back(combi6->combi[3]);
            }
          }
	  double Esum2gg3 = EMList[combi6->combi[0]]->lorentzMomentum().E() + EMList[combi6->combi[1]]->lorentzMomentum().E();
          if (AnEtaPrim3 && Esum2gg3 > 500.0e-3) {
	    if (Chi2_2pi0eta_2 < bestChi0EtaPrim) {
              bestChi0EtaPrim = Chi2_2pi0eta_2;
	      Indexprim6gList.clear();
              Indexprim6gList.push_back(combi6->combi[2]);
              Indexprim6gList.push_back(combi6->combi[3]);
              Indexprim6gList.push_back(combi6->combi[4]);
              Indexprim6gList.push_back(combi6->combi[5]);
              Indexprim6gList.push_back(combi6->combi[0]);
              Indexprim6gList.push_back(combi6->combi[1]);
            }
          }
        }
      }
    }
  }
  if(Index6gList.size()>0)
    Indexprim6gList.clear();
}
