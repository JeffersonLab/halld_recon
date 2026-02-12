/**************************************************************************                                                             
* HallD software                                                          * 
* Copyright(C) 2025       GlueX Collaboration                             * 
*                                                                         *                                                                
* Author: The GlueX Collaborations                                        *                                                                
* Contributors: Igal Jaegle                                               *                                                               
*                                                                         *
*                                                                         *   
* This software is provided "as is" without any warranty.                 *
**************************************************************************/

#include "JEventProcessor_cal_cal.h"
#include <DANA/DEvent.h>

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_cal_cal());
}
} // "C"


//------------------
// init
//------------------
void JEventProcessor_cal_cal::Init()
{
  auto app = GetApplication();
  m_TIME_CUT_RF_TAG = 2.004;
  m_TIME_CUT_RF_SC_MIN = 1.0;
  m_TIME_CUT_RF_SC_MAX = 7.0;
  m_ENERGY_CUT_SC = 0.002;
  m_TIME_CUT_RF_TOF = 0.5;
  m_TIME_CUT_RF_ECAL = 2.004;
  m_TIME_CUT_RF_FCAL = 2.004;
  m_FCAL1 = 0;
  app->SetDefaultParameter("cal_cal:FCAL1", m_FCAL1);
  app->SetDefaultParameter("cal_cal:TIME_CUT_RF_TAG", m_TIME_CUT_RF_TAG);
  app->SetDefaultParameter("cal_cal:TIME_CUT_RF_SC_MIN", m_TIME_CUT_RF_SC_MIN);
  app->SetDefaultParameter("cal_cal:TIME_CUT_RF_SC_MAX", m_TIME_CUT_RF_SC_MAX);
  app->SetDefaultParameter("cal_cal:ENERGY_CUT_SC", m_ENERGY_CUT_SC);
  app->SetDefaultParameter("cal_cal:TIME_CUT_RF_TOF", m_TIME_CUT_RF_TOF);
  app->SetDefaultParameter("cal_cal:TIME_CUT_RF_ECAL", m_TIME_CUT_RF_ECAL);
  app->SetDefaultParameter("cal_cal:TIME_CUT_RF_FCAL", m_TIME_CUT_RF_FCAL);

  lockService = app->GetService<JLockService>();
  
  //TH1::SetDefaultSumw2();
  //TH2::SetDefaultSumw2();
  TDirectory * dir_cal_cal = new TDirectoryFile("cal_cal", "cal_cal");
  dir_cal_cal->cd();
  h_ecal_e_v_ce = new TH2F("ecal_e_v_ce", ";#font[42]{E_{cluster} [GeV]};#font[42]{E_{shower} / E_{energy}};#font[42]{Events #}", 1200, 0., 12., 400, 0.8, 1.2);
  h_ecal_e_ratio = new TH2F("ecal_e_ratio", ";#font[42]{E_{cluster} [GeV]};#font[42]{E_{max}^{cluster} / E_{cluster}};#font[42]{Events #}", 1200, 0., 12., 500, 0.0, 1.0);
  h_fcal_e_v_ce = new TH2F("fcal_e_v_ce", ";#font[42]{E_{cluster} [GeV]};#font[42]{E_{shower} / E_{energy}};#font[42]{Events #}", 1200, 0., 12., 400, 0.8, 1.2);
  h_fcal_e_ratio = new TH2F("fcal_e_ratio", ";#font[42]{E_{cluster} [GeV]};#font[42]{E_{max}^{cluster} / E_{cluster}};#font[42]{Events #}", 1200, 0., 12., 500, 0.0, 1.0);
  h_ecal_cluster_size = new TH1F("ecal_cluster_size", ";#font[42]{Number of hits making a cluster};#font[42]{Events #}", 100, -0.5, 99.5);
  h_fcal_cluster_size = new TH1F("fcal_cluster_size", ";#font[42]{Number of hits making a cluster};#font[42]{Events #}", 100, -0.5, 99.5);
  //h_ecal_blk_v_layer = new TH2F("ecal_blk_v_layer", ";#font[42]{Layer #};#font[42]{Blk #}", 31, -0.5, 30.5, 1600, -0.5, 1599.5);
  //h_fcal_blk_v_layer = new TH2F("fcal_blk_v_layer", ";#font[42]{Layer #};#font[42]{Blk #}", 31, -0.5, 30.5, 2800, -0.5, 2799.5);
  //h_ecal_layer = new TH1F("ecal_layer", ";#font[42]{Layer #};#font[42]{Events #}", 21, -0.5, 20.5);
  //h_fcal_layer = new TH1F("fcal_layer", ";#font[42]{Layer #};#font[42]{Events #}", 31, -0.5, 30.5);
  //h_ecal_ring = new TH1F("ecal_ring", ";#font[42]{Ring #};#font[42]{Events #}", 31, -0.5, 30.5);
  //h_fcal_ring = new TH1F("fcal_ring", ";#font[42]{Ring #};#font[42]{Events #}", 31, -0.5, 30.5);
  h_fcal_to = new TProfile("fcal_to", "Current TO Constants", 2800, -0.5, 2799.5);
  h_fcal_gains = new TProfile("fcal_gains", "Current Gain Constants", 2800, -0.5, 2799.5);
  h_fcal_qualities = new TProfile("fcal_qualities", "Current Block Qualities", 2800, -0.5, 2799.5);
  h_ecal_to = new TProfile("ecal_to", "Current TO Constants", 1600, -0.5, 1599.5);
  h_ecal_gains = new TProfile("ecal_gains", "Current Gain Constants", 1600, -0.5, 1599.5);
  h_ecal_qualities = new TProfile("ecal_qualities", "Current Block Qualities", 1600, -0.5, 1599.5);
  for (int i = 0; i < 6; i ++) {
    h_ecal_cr_ring[i] = new TH2F(Form("ecal_cr_ring_%d", i), ";#font[42]{Column #};#font[42]{Row #};#font[42]{Events #}", 40, -20, 20, 40, -20, 20);
    h_fcal_cr_ring[i] = new TH2F(Form("fcal_cr_ring_%d", i), ";#font[42]{Column #};#font[42]{Row #};#font[42]{Events #}", 59, -30, 30, 59, -30, 30); 
  }
  for (int i = 0; i < 14; i ++) {
    h_ecal_xy[i] = new TH2F(Form("ecal_xy_%d", i), ";#font[42]{x [cm]};#font[42]{y [cm]};#font[42]{Events #}", 500, -50., 50., 500, -50., 50.);
    h_ecal_cr[i] = new TH2F(Form("ecal_cr_%d", i), ";#font[42]{Column #};#font[42]{Row #};#font[42]{Events #}", 40, -20, 20, 40, -20, 20);

    h_fcal_xy[i] = new TH2F(Form("fcal_xy_%d", i), ";#font[42]{x [cm]};#font[42]{y [cm]};#font[42]{Events #}", 500, -125., 125., 500, -125., 125.);
    h_fcal_cr[i] = new TH2F(Form("fcal_cr_%d", i), ";#font[42]{Column #};#font[42]{Row #};#font[42]{Events #}", 59, -30, 30, 59, -30, 30); 
  }
  for (int i = 0; i < 5; i ++) {
    h_ecal_time[i] = new TH2F(Form("ecal_time_%d", i), ";#font[42]{Block #};#font[42]{t_{RF} - t_{ECAL} [ns]};#font[42]{Events #}", 1600, -0.5, 1599.5, 5000, -250., 250.);
    h_fcal_time[i] = new TH2F(Form("fcal_time_%d", i), ";#font[42]{Block #};#font[42]{t_{RF} - t_{FCAL} [ns]};#font[42]{Events #}", 2800, -0.5, 2799.5, 5000, -250., 250.);
    h_ecal_v_ecal_time[i] = new TH2F(Form("ecal_v_ecal_time_%d", i), ";#font[42]{Block #};#font[42]{t_{ECAL}^{1} - t_{ECAL}^{2} [ns]};#font[42]{Events #}", 1600, -0.5, 1599.5, 5000, -250., 250.);
    h_fcal_v_fcal_time[i] = new TH2F(Form("fcal_v_fcal_time_%d", i), ";#font[42]{Block #};#font[42]{t_{FCAL}^{1} - t_{FCAL}^{2} [ns]};#font[42]{Events #}", 2800, -0.5, 2799.5, 5000, -250., 250.);
    h_ecal_time[i]->Sumw2();
    h_fcal_time[i]->Sumw2();
    h_ecal_v_ecal_time[i]->Sumw2();
    h_fcal_v_fcal_time[i]->Sumw2();
    h_ecal_ctime[i] = new TH2F(Form("ecal_ctime_%d", i), ";#font[42]{Block #};#font[42]{t_{RF} - t_{ECAL} [ns]};#font[42]{Events #}", 1600, -0.5, 1599.5, 5000, -250., 250.);
    h_fcal_ctime[i] = new TH2F(Form("fcal_ctime_%d", i), ";#font[42]{Block #};#font[42]{t_{RF} - t_{FCAL} [ns]};#font[42]{Events #}", 2800, -0.5, 2799.5, 5000, -250., 250.);
    h_ecal_v_ecal_ctime[i] = new TH2F(Form("ecal_v_ecal_ctime_%d", i), ";#font[42]{Block #};#font[42]{t_{ECAL}^{1} - t_{ECAL}^{2} [ns]};#font[42]{Events #}", 1600, -0.5, 1599.5, 5000, -250., 250.);
    h_fcal_v_fcal_ctime[i] = new TH2F(Form("fcal_v_fcal_ctime_%d", i), ";#font[42]{Block #};#font[42]{t_{FCAL}^{1} - t_{FCAL}^{2} [ns]};#font[42]{Events #}", 2800, -0.5, 2799.5, 5000, -250., 250.);
    h_ecal_ctime[i]->Sumw2();
    h_fcal_ctime[i]->Sumw2();
    h_ecal_v_ecal_ctime[i]->Sumw2();
    h_fcal_v_fcal_ctime[i]->Sumw2();
  }
  for (int i = 0; i < 6; i ++) {
    h_fcal_time_v_bit[i] = new TH2F(Form("fcal_time_v_bit_%d", i), ";#font[42]{Block #};#font[42]{t_{RF} - t_{FCAL} [ns]};#font[42]{Events #}", 2800, -0.5, 2799.5, 1000, -50., 50.);
    h_fcal_ctime_v_bit[i] = new TH2F(Form("fcal_tcime_v_bit_%d", i), ";#font[42]{Block #};#font[42]{t_{RF} - t_{FCAL} [ns]};#font[42]{Events #}", 2800, -0.5, 2799.5, 1000, -50., 50.);
  }
  for (int i = 0; i < 7; i ++) {
    h_ecal_mgg_v_blk[i] = new TH2F(Form("ecal_mgg_v_blk_%d", i), ";#font[42]{Block #};#font[42]{m_{#gamma#gamma} [GeV/c^{2}]};#font[42]{Events #}", 1600, -0.5, 1599.5, 1200, 0., 1.2);
    h_efcal_mgg_v_blk[i] = new TH2F(Form("efcal_mgg_v_blk_%d", i), ";#font[42]{Block #};#font[42]{m_{#gamma#gamma} [GeV/c^{2}]};#font[42]{Events #}", 1600, -0.5, 1599.5, 1200, 0., 1.2);
    h_fecal_mgg_v_blk[i] = new TH2F(Form("fecal_mgg_v_blk_%d", i), ";#font[42]{Block #};#font[42]{m_{#gamma#gamma} [GeV/c^{2}]};#font[42]{Events #}", 2800, -0.5, 2799.5, 1200, 0., 1.2);
    h_fcal_mgg_v_blk[i] = new TH2F(Form("fcal_mgg_v_blk_%d", i), ";#font[42]{Block #};#font[42]{m_{#gamma#gamma} [GeV/c^{2}]};#font[42]{Events #}", 2800, -0.5, 2799.5, 1200, 0., 1.2);
    h_ecal_mgg_v_blk[i]->Sumw2();
    h_fcal_mgg_v_blk[i]->Sumw2();
    h_efcal_mgg_v_blk[i]->Sumw2();
    h_fecal_mgg_v_blk[i]->Sumw2();
  }
  for (int i = 0; i < 7; i ++) {
    h_fcal_fdc_mgg_v_blk[i] = new TH2F(Form("fcal_fdc_mgg_v_blk_%d", i), ";#font[42]{Block #};#font[42]{m_{#gamma#gamma} [GeV/c^{2}]};#font[42]{Events #}", 2800, -0.5, 2799.5, 1200, 0., 1.2);
    //h_efcal_fdc_mgg_v_blk[i] = new TH2F(Form("fcal_efdc_mgg_v_blk_%d", i), ";#font[42]{Block #};#font[42]{m_{#gamma#gamma} [GeV/c^{2}]};#font[42]{Events #}", 1600, -0.5, 1599.5, 1200, 0., 1.2);
    h_fecal_fdc_mgg_v_blk[i] = new TH2F(Form("fecal_fdc_mgg_v_blk_%d", i), ";#font[42]{Block #};#font[42]{m_{#gamma#gamma} [GeV/c^{2}]};#font[42]{Events #}", 2800, -0.5, 2799.5, 1200, 0., 1.2);
    h_fecal_fdc_mgg_v_blk[i]->Sumw2();
    h_fcal_fdc_mgg_v_blk[i]->Sumw2();
  }
  //h_ecal_mgg_v_layer = new TH2F("ecal_mgg_v_layer", ";#font[42]{Layer #};#font[42]{m_{#gamma#gamma} [GeV/c^{2}]};#font[42]{Events #}", 21, -0.5, 20.5, 1200, 0., 1.2);
  //h_fcal_mgg_v_layer = new TH2F("fcal_mgg_v_layer", ";#font[42]{Layer #};#font[42]{m_{#gamma#gamma} [GeV/c^{2}]};#font[42]{Events #}", 21, -0.5, 20.5, 1200, 0., 1.2);
  //h_ecal_mgg_v_ring = new TH2F("ecal_mgg_v_ring", ";#font[42]{Ring #};#font[42]{m_{#gamma#gamma} [GeV/c^{2}]};#font[42]{Events #}", 31, -0.5, 30.5, 1200, 0., 1.2);
  //h_fcal_mgg_v_ring = new TH2F("fcal_mgg_v_ring", ";#font[42]{Ring #};#font[42]{m_{#gamma#gamma} [GeV/c^{2}]};#font[42]{Events #}", 21, -0.5, 20.5, 1200, 0., 1.2);
  h_tag_time_rf_v_e = new TH2F("tag_time_rf_v_e", ";#font[42]{t_{RF} - t_{TAG} [ns]};#font[42]{E_{#gamma}^{beam} [GeV]};#font[42]{Events #}", 700, -35., 35., 500, 2., 12.);

  h_tag_time_rf_v_e_fdc = new TH2F("tag_time_rf_v_e_fdc", ";#font[42]{t_{RF} - t_{TAG} [ns]};#font[42]{E_{#gamma}^{beam} [GeV]};#font[42]{Events #}", 700, -35., 35., 500, 2., 12.);
  
  h_tag_time_fcal_v_e = new TH2F("tag_time_fcal_v_e", ";#font[42]{t_{FCAL} - t_{TAG} [ns]};#font[42]{E_{#gamma}^{beam} [GeV]};#font[42]{Events #}", 700, -35., 35., 500, 2., 12.);
  h_tag_time_ecal_v_e = new TH2F("tag_time_ecal_v_e", ";#font[42]{t_{ECAL} - t_{TAG} [ns]};#font[42]{E_{#gamma}^{beam} [GeV]};#font[42]{Events #}", 700, -35., 35., 500, 2., 12.);
  h_trg_bit = new TH1F ("trg_bit", ";#font[42]{Trigger bit #};#font[42]{Events #}", 100, -0.5, 99.5);
  for (int i = 0; i < 4; i ++) {
    h_esum_bcal[i] = new TH1F(Form("esum_bcal_%d", i), ";#font[42]{E_{sum}^{BCAL} [GeV]};#font[42]{Events #}", 1200, 0., 12.);
    h_esum_ecal[i] = new TH1F(Form("esum_ecal_%d", i), ";#font[42]{E_{sum}^{ECAL} [GeV]};#font[42]{Events #}", 1200, 0., 12.);
    h_esum_fcal[i] = new TH1F(Form("esum_fcal_%d", i), ";#font[42]{E_{sum}^{FCAL} [GeV]};#font[42]{Events #}", 1200, 0., 12.);
    h_esum_efcal[i] = new TH1F(Form("esum_efcal_%d", i), ";#font[42]{E_{sum}^{ECAL}+E_{sum}^{FCAL} [GeV]};#font[42]{Events #}", 1200, 0., 12.);
    h_esum_trg[i] = new TH2F(Form("esum_trg_%d", i), ";#font[42]{E_{sum}^{FCAL} + E_{sum}^{ECAL} [GeV]};#font[42]{E_{sum}^{BCAL} [GeV]};#font[42]{Events #}", 500, 0., 10., 500, 0., 10.);
  }
  h_tof_time = new TH1F("tof_time", ";#font[42]{t_{RF} - t_{TOF} [ns]};#font[42]{Events #}", 1000, -50., 50.);
  h_sc_time_v_e = new TH2F("sc_time_v_e", ";#font[42]{t_{RF} - t_{SC} [ns]};#font[42]{Energy [MeV]};#font[42]{Events #}", 1000, -50., 50., 1000, 0., 0.01);
    
  for (int i = 0; i < 5; i ++) {
    h_ecal_im[i] = new TH1F(Form("ecal_im_%d", i), ";#font[42]{m [GeV]};#font[42]{Events #}", 5000, 0., 2.);
    h_fcal_im[i] = new TH1F(Form("fcal_im_%d", i), ";#font[42]{m [GeV]};#font[42]{Events #}", 5000, 0., 2.);
  }
  for (int i = 0; i < 25; i ++) {
    h_ecal_nl[i] = new TH2F(Form("ecal_nl_%d", i), ";#font[42]{E_{cluster}^{bachelor} [GeV]};#font[42]{#it{m}_{#gamma#gamma} [GeV/#it{c}^{2}]};#font[42]{Events #}", 900, 0., 9., 1000, 0., 0.5);
    h_fcal_nl[i] = new TH2F(Form("fcal_nl_%d", i), ";#font[42]{E_{cluster}^{bachelor} [GeV]};#font[42]{#it{m}_{#gamma#gamma} [GeV/#it{c}^{2}]};#font[42]{Events #}", 900, 0., 9., 1000, 0., 0.5);
    for (int j = 0; j < 3; j ++) {
      h_ecal_wnl[j][i] = new TH2F(Form("ecal_wnl_%d_%d", j, i), ";#font[42]{E_{cluster}^{bachelor} [GeV]};#font[42]{#it{m}_{#gamma#gamma} [GeV/#it{c}^{2}]};#font[42]{Events #}", 900, 0., 9., 1000, 0., 0.5);
      h_ecal_snl[j][i] = new TH2F(Form("ecal_snl_%d_%d", j, i), ";#font[42]{E_{cluster}^{bachelor} [GeV]};#font[42]{#it{m}_{#gamma#gamma} [GeV/#it{c}^{2}]};#font[42]{Events #}", 900, 0., 9., 1000, 0., 0.5);
      h_fcal_wnl[j][i] = new TH2F(Form("fcal_wnl_%d_%d", j, i), ";#font[42]{E_{cluster}^{bachelor} [GeV]};#font[42]{#it{m}_{#gamma#gamma} [GeV/#it{c}^{2}]};#font[42]{Events #}", 900, 0., 9., 1000, 0., 0.5);
      h_fcal_snl[j][i] = new TH2F(Form("fcal_snl_%d_%d", j, i), ";#font[42]{E_{cluster}^{bachelor} [GeV]};#font[42]{#it{m}_{#gamma#gamma} [GeV/#it{c}^{2}]};#font[42]{Events #}", 900, 0., 9., 1000, 0., 0.5);
    }
  }

  dir_cal_cal->cd("../");
  return;
}

//------------------
// brun
//------------------
void JEventProcessor_cal_cal::BeginRun(const std::shared_ptr<const JEvent>& event) {
  
  DGeometry*   dgeom = DEvent::GetDGeometry(event);
  
  if( dgeom ){
    dgeom->GetTargetZ( m_beamZ );
    dgeom->GetFCALPosition( m_fcalX, m_fcalY, m_fcalZ );
    dgeom->GetECALPosition( m_ecalX, m_ecalY, m_ecalZ );
  } else{
    cerr << "No geometry accessbile to compton_analysis plugin." << endl;
    return;
  }
 
  dgeom->GetStartCounterGeom(sc_pos,sc_norm);
  
  event->GetSingle(fcalGeom);
  if (m_FCAL1 == 0) event->GetSingle(ecalGeom);
  
  auto app = GetApplication();
  JCalibration *jcalib = app->GetService<JCalibrationManager>()->GetJCalibration(event->GetRunNumber());
  std::map<string, float> beam_spot;
  jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  m_beamX  =  beam_spot.at("x");
  m_beamY  =  beam_spot.at("y");
  m_rnb = event->GetRunNumber();
  m_time_rf_offset = 0;
  //if (m_rnb >= 130910) m_time_rf_offset = - 20;
  TargetN = Proton;
  
  vector< double > ecal_raw_to;
  jcalib->Get("/ECAL/timing_offsets", ecal_raw_to);
  for (unsigned int i = 0; i < ecal_raw_to.size(); i++) h_ecal_to->Fill(i, ecal_raw_to[i]);
  vector< double > ecal_raw_gains;
  jcalib->Get("/ECAL/gains", ecal_raw_gains);
  for (unsigned int i = 0; i < ecal_raw_gains.size(); i++) h_ecal_gains->Fill(i, ecal_raw_gains[i]);
  vector< double > fcal_raw_to;
  jcalib->Get("/FCAL/timing_offsets", fcal_raw_to);
  for (unsigned int i = 0; i < fcal_raw_to.size(); i++) h_fcal_to->Fill(i, fcal_raw_to[i]);
  vector< double > fcal_raw_gains;
  jcalib->Get("/FCAL/gains", fcal_raw_gains);
  for (unsigned int i = 0; i < fcal_raw_gains.size(); i++) h_fcal_gains->Fill(i, fcal_raw_gains[i]);
  vector< int > fcal_raw_qualities;
  jcalib->Get("/FCAL/block_quality", fcal_raw_qualities);
  for (unsigned int i = 0; i < fcal_raw_qualities.size(); i++) h_fcal_qualities->Fill(i, fcal_raw_qualities[i]);
  
  return;
}

//------------------
// evnt
//------------------
void JEventProcessor_cal_cal::Process(const std::shared_ptr<const JEvent>& event) {
  //auto lockService = DEvent::GetLockService(event);
  
  auto locL1Triggers = event->Get<DL1Trigger>();
  auto locNeutralParticles = event->Get<DNeutralParticle>();
  auto locECALShowers = event->Get<DECALShower>();
  auto locFCALShowers = event->Get<DFCALShower>();
  auto locTOFPoints = event->Get<DTOFPoint>();
  auto locSCHits = event->Get<DSCHit>();
  auto locBeamPhotons = event->Get<DBeamPhoton>();
  auto locBCALHits = event->Get<DBCALHit>();
  auto locECALHits = event->Get<DECALHit>();
  auto locFCALHits = event->Get<DFCALHit>();
  auto locEventRFBunches = event->Get<DEventRFBunch>();
  double locRFTime = locEventRFBunches.empty() ? 0.0 : locEventRFBunches[0]->dTime;
  //int laymin = 10;
  //if (event->GetRunNumber() < 130000) laymin = 2;
  
  lockService->RootFillLock(this);
  //locRFTime += m_time_rf_offset;
  //cout << "locRF time " << locRFTime << " offset " << m_time_rf_offset << endl;
  int trig_bit[33];
  if (locL1Triggers.size() > 0) {
    for (unsigned int bit = 0; bit < 32; bit ++) {
      trig_bit[bit + 1] = (locL1Triggers[0]->trig_mask & (1 << bit)) ? 1 : 0;
      if (trig_bit[bit + 1] == 1) h_trg_bit->Fill(Float_t(bit+1));
    }
  }
  
  Float_t BCAL_trg_Esum = 0;
  Float_t BCAL_trg_Esum_raw = 0;
  for (vector<const DBCALHit*>::const_iterator hit  = locBCALHits.begin(); hit != locBCALHits.end(); hit++ ) {
    BCAL_trg_Esum_raw += (**hit).E;
    if ((**hit).E > 0.013195)
      BCAL_trg_Esum += (**hit).E;
  }
  Float_t FCAL_trg_Esum = 0;
  Float_t FCAL_trg_Esum_raw = 0;
  for (vector<const DFCALHit*>::const_iterator hit  = locFCALHits.begin(); hit != locFCALHits.end(); hit++ ) {
    FCAL_trg_Esum_raw += (**hit).E;
    if ((**hit).E > 0.130)
      FCAL_trg_Esum += (**hit).E;
  }
  Float_t ECAL_trg_Esum = 0;
  Float_t ECAL_trg_Esum_raw = 0;
  for (vector<const DECALHit*>::const_iterator hit  = locECALHits.begin(); hit != locECALHits.end(); hit++ ) {
    ECAL_trg_Esum_raw += (**hit).E;
    if ((**hit).E > 0.0325)
      ECAL_trg_Esum += (**hit).E;
  }
  
  int sc_trg_nb = 0;
  for (unsigned int i = 0; i < locSCHits.size(); i ++) {
    const DSCHit *schits = locSCHits[i];
    if ((schits->dE > 0.0002) && (1.0 < (schits->t - locRFTime) && (schits->t - locRFTime) < 7.0))
      sc_trg_nb ++;
  }
  
  //bool b_good_ecalfcal_energy_sum = FCAL_trg_Esum + ECAL_trg_Esum < 10;
  //bool b_good_fcal_shower_nb = 2 <= locFCALShowers.size() && locFCALShowers.size() <= 8;
  //bool b_good_ecalfcal_shower_nb = 2 <= (locECALShowers.size() + locFCALShowers.size()) && (locECALShowers.size() + locFCALShowers.size()) <= 8;
  h_esum_bcal[0]->Fill(BCAL_trg_Esum_raw);
  h_esum_ecal[0]->Fill(ECAL_trg_Esum_raw);
  h_esum_fcal[0]->Fill(FCAL_trg_Esum_raw);
  h_esum_efcal[0]->Fill(ECAL_trg_Esum_raw + FCAL_trg_Esum_raw);
  h_esum_trg[0]->Fill(FCAL_trg_Esum_raw + ECAL_trg_Esum_raw, BCAL_trg_Esum_raw);
  if (trig_bit[1] == 1 || trig_bit[3] == 1) {
    h_esum_bcal[1]->Fill(BCAL_trg_Esum_raw);
    h_esum_ecal[1]->Fill(ECAL_trg_Esum_raw);
    h_esum_fcal[1]->Fill(FCAL_trg_Esum_raw);
    h_esum_efcal[1]->Fill(ECAL_trg_Esum_raw + FCAL_trg_Esum_raw);
    h_esum_trg[1]->Fill(FCAL_trg_Esum_raw + ECAL_trg_Esum_raw, BCAL_trg_Esum_raw);
    if (trig_bit[1] == 1) {
      h_esum_bcal[2]->Fill(BCAL_trg_Esum_raw);
      h_esum_ecal[2]->Fill(ECAL_trg_Esum_raw);
      h_esum_fcal[2]->Fill(FCAL_trg_Esum_raw);
      h_esum_efcal[2]->Fill(ECAL_trg_Esum_raw + FCAL_trg_Esum_raw);
      h_esum_trg[2]->Fill(FCAL_trg_Esum_raw + ECAL_trg_Esum_raw, BCAL_trg_Esum_raw);
    }
    if (trig_bit[3] == 1) {
      h_esum_bcal[3]->Fill(BCAL_trg_Esum_raw);
      h_esum_ecal[3]->Fill(ECAL_trg_Esum_raw);
      h_esum_fcal[3]->Fill(FCAL_trg_Esum_raw);
      h_esum_efcal[3]->Fill(ECAL_trg_Esum_raw + FCAL_trg_Esum_raw);
      h_esum_trg[3]->Fill(FCAL_trg_Esum_raw + ECAL_trg_Esum_raw, BCAL_trg_Esum_raw);
    }
  }

  // Vertex info
  DVector3 vertex;
  vertex.SetXYZ(m_beamX, m_beamY, m_beamZ);
  m_2ndfdc_package = 240;
  DVector3 vertex_1stFDC_package(m_beamX, m_beamY, m_2ndfdc_package);
  
  //bool b_inner_layers = false;
  float m_weight = 0;
  vector<const DECALShower *> Photons_ecal_list; Photons_ecal_list.clear();
  vector<const DFCALShower *> Photons_fcal_list; Photons_fcal_list.clear();
  vector<TLorentzVector> photons_ecal_list; photons_ecal_list.clear();
  vector<TLorentzVector> photons_fcal_list; photons_fcal_list.clear();
  
  //if (m_FCAL1 == 0) {
  for (unsigned int i = 0; i < locECALShowers.size(); i++) {
    double e1 = locECALShowers[i]->E;
    DVector3 position1(0, 0, 0);
    position1 = locECALShowers[i]->pos - vertex;
    double r1 = position1.Mag();
    double t1 = locECALShowers[i]->t - (r1 / TMath::C() * 1e7);
    double p1x = e1 * sin(position1.Theta()) * cos(position1.Phi());
    double p1y = e1 * sin(position1.Theta()) * sin(position1.Phi());
    double p1z = e1 * cos(position1.Theta());
    DLorentzVector photon1P4(p1x, p1y, p1z, e1);

    DVector3 position1_fdc(0, 0, 0);
    position1_fdc = locECALShowers[i]->pos - vertex_1stFDC_package;
    double p1x_fdc = e1 * sin(position1_fdc.Theta()) * cos(position1_fdc.Phi());
    double p1y_fdc = e1 * sin(position1_fdc.Theta()) * sin(position1_fdc.Phi());
    double p1z_fdc = e1 * cos(position1_fdc.Theta());
    DLorentzVector photon1P4_fdc(p1x_fdc, p1y_fdc, p1z_fdc, e1);
    
    double diff_t1 = locRFTime - t1;
    //float xc1 = position1.X();
    //float yc1 = position1.Y();
    const DECALCluster * ecalCluster1;
    locECALShowers[i]->GetSingle(ecalCluster1);
    vector<const DECALHit*> ecalHit1;
    ecalCluster1->Get(ecalHit1);
    if (ecalHit1.size() < 2) continue;
    double ce1 = ecalCluster1->E;
    double ce1_max = 0;
    double ct1_max = -100;
    for (int j = 0; j < (int) ecalHit1.size(); j ++) if (ce1_max < ecalHit1[j]->E) {
	ce1_max = ecalHit1[j]->E;
	ct1_max = ecalHit1[j]->t - (r1 / TMath::C() * 1e7);
      }
    double diff_ct1 = locRFTime - ct1_max;
    int ch1 = ecalCluster1->channel_Emax;
    h_ecal_e_v_ce->Fill(ce1, ce1 / e1);
    h_ecal_e_ratio->Fill(ce1, ce1_max / ce1);
    int col1 = ecalGeom->column(ch1);
    int row1 = ecalGeom->row(ch1);
    //int lay1 = get_ecal_layer(row1, col1);
    //if (lay1 < 3) b_inner_layers = true;
    float xc1 = ecalCluster1->x;
    float yc1 = ecalCluster1->y;
    double radiusc1 = sqrt(xc1 * xc1 + yc1 * yc1);;
    int ring1_nb = (int) (radiusc1 / (5 * k_cm));
    double frac1 = ce1_max / ce1;
    double thres_frac1 = 0;
    if (0 <= ring1_nb && ring1_nb <= 5) thres_frac1 = 0.5;
    if (6 <= ring1_nb && ring1_nb <= 13) thres_frac1 = 0.4;
    if (14 <= ring1_nb && ring1_nb <= 20) thres_frac1 = 0.3;
    if (21 <= ring1_nb && ring1_nb <= 25) thres_frac1 = 0.25;
    
    //h_ecal_blk_v_layer->Fill(lay1, ch1);
    //h_ecal_ring->Fill(ring1_nb);
    //h_ecal_layer->Fill(lay1);
    //h_ecal_cr_ring[lay1]->Fill(col1 - 20, row1 - 20);
    //double phi_ecal1 = position1.Phi();
    double delta_x_min1 = 1000.;
    double delta_y_min1 = 1000.;
    //double delta_phi_min1 = 1000.;
    //int sc_match1 = check_SC_match(phi_ecal1, locRFTime, locSCHits, delta_phi_min1);
    int tof_match1 = check_TOF_match(position1, locRFTime, vertex, locTOFPoints, delta_x_min1, delta_y_min1);
    //bool b_tof_veto1 = delta_x_min1 > 8 && delta_y_min1 > 8;
    //if (!b_tof_veto1) continue;
    //if (tof_match1 != 0) continue;
    tof_match1 = 0;
    if (delta_x_min1 < 4 && delta_y_min1 < 4) {
      tof_match1 = 1;
    }

    if (tof_match1 == 0 && fabs(diff_t1) < 3.005) {
      Photons_ecal_list.push_back(locECALShowers[i]);
      photons_ecal_list.push_back(photon1P4);
    }
    
    h_ecal_xy[0]->Fill(xc1, yc1);
    h_ecal_cr[0]->Fill(col1 - 20, row1 - 20);
    for (int k = 1; k < 11; k ++) {
      if (e1 >= ((double) k)) {
	h_ecal_xy[k]->Fill(xc1, yc1);
	h_ecal_cr[k]->Fill(col1 - 20, row1 - 20);
      }
    }
    h_ecal_time[0]->Fill(ch1, diff_t1);
    h_ecal_ctime[0]->Fill(ch1, diff_ct1);
    for (unsigned int j = i + 1; j < locECALShowers.size(); j++) {
      double e2 = locECALShowers[j]->E;
      DVector3 position2(0, 0, 0);
      position2 = locECALShowers[j]->pos - vertex;
      double r2 = position2.Mag();
      double t2 = locECALShowers[j]->t - (r2 / TMath::C() * 1e7);
      double p2x = e2 * sin(position2.Theta()) * cos(position2.Phi());
      double p2y = e2 * sin(position2.Theta()) * sin(position2.Phi());
      double p2z = e2 * cos(position2.Theta());
      DLorentzVector photon2P4(p2x, p2y, p2z, e2);
      double diff_t2 = locRFTime - t2;
      //float xc2 = position2.X();
      //float yc2 = position2.Y();
      const DECALCluster * ecalCluster2;
      locECALShowers[j]->GetSingle(ecalCluster2);
      vector<const DECALHit*> ecalHit2;
      ecalCluster2->Get(ecalHit2);
      if (ecalHit2.size() < 2) continue;
      //double ce2 = ecalCluster2->E;
      double ce2_max = 0;
      double ct2_max = -100;
      for (int k = 0; k < (int) ecalHit2.size(); k ++) if (ce2_max < ecalHit2[k]->E) {
	  ce2_max = ecalHit2[k]->E;
	  ct2_max = ecalHit2[k]->t - (r2 / TMath::C() * 1e7);
	}
      double diff_ct2 = locRFTime - ct2_max;
      //const vector< DECALCluster::DECALClusterHit_t > ECALhitVector2 = ecalCluster2->HitInfo();
      //if (ECALhitVector2.size() < 1) continue;
      int ch2 = ecalCluster2->channel_Emax;
      int col2 = ecalGeom->column(ch2);
      int row2 = ecalGeom->row(ch2);
      //int lay2 = get_ecal_layer(row2, col2);
      float xc2 = ecalCluster2->x;
      float yc2 = ecalCluster2->y;
      double radiusc2 = sqrt(xc2 * xc2 + yc2 * yc2);;
      int ring2_nb = (int) (radiusc2 / (5 * k_cm));
      double frac2 = ce1_max / ce1;
      double thres_frac2 = 0;
      if (0 <= ring2_nb && ring2_nb <= 5) thres_frac2 = 0.5;
      if (6 <= ring2_nb && ring2_nb <= 13) thres_frac2 = 0.4;
      if (14 <= ring2_nb && ring2_nb <= 20) thres_frac2 = 0.3;
      if (21 <= ring2_nb && ring2_nb <= 25) thres_frac2 = 0.25;
      
      //double phi_ecal2 = position2.Phi();
      double delta_x_min2 = 1000.;
      double delta_y_min2 = 1000.;
      //double delta_phi_min2 = 1000.;
      //int sc_match2 = check_SC_match(phi_ecal2, locRFTime, locSCHits, delta_phi_min2);
      int tof_match2 = check_TOF_match(position2, locRFTime, vertex, locTOFPoints, delta_x_min2, delta_y_min2);
      //bool b_tof_veto2 = delta_x_min2 > 8 && delta_y_min2 > 8;
      //if (!b_tof_veto2) continue;
      //if (tof_match2 != 0) continue;
      tof_match2 = 0;
      if (delta_x_min2 < 6 && delta_y_min2 < 6) {
	tof_match2 = 1;
      }
      
      DLorentzVector pi0P4 = photon1P4 + photon2P4;
      if (pi0P4.M() < 0.02) continue;
      
      h_ecal_time[1]->Fill(ch1, diff_t1);
      h_ecal_time[1]->Fill(ch2, diff_t2);
      h_ecal_ctime[1]->Fill(ch1, diff_ct1);
      h_ecal_ctime[1]->Fill(ch2, diff_ct2);
      
      if (e1 > 0.5 && e2 > 0.5 && locECALShowers.size() < 10) {
	h_ecal_time[2]->Fill(ch1, diff_t1);
	h_ecal_time[2]->Fill(ch2, diff_t2);
	h_ecal_ctime[2]->Fill(ch1, diff_ct1);
	h_ecal_ctime[2]->Fill(ch2, diff_ct2);	
	if (tof_match1 == 0 && tof_match2 == 0) {
	  h_ecal_time[3]->Fill(ch1, diff_t1);
	  h_ecal_time[3]->Fill(ch2, diff_t2);
	  h_ecal_ctime[3]->Fill(ch1, diff_ct1);
	  h_ecal_ctime[3]->Fill(ch2, diff_ct2);
	  if (0.11 <= pi0P4.M() && pi0P4.M() <= 0.16) {
	    h_ecal_time[4]->Fill(ch1, diff_t1);
	    h_ecal_time[4]->Fill(ch2, diff_t2);
	    h_ecal_ctime[4]->Fill(ch1, diff_ct1);
	    h_ecal_ctime[4]->Fill(ch2, diff_ct2);
	  }
	}
      }
      
      h_ecal_v_ecal_time[0]->Fill(ch1, diff_t1 - diff_t2);
      h_ecal_v_ecal_time[0]->Fill(ch2, diff_t2 - diff_t1);
      h_ecal_v_ecal_ctime[0]->Fill(ch1, diff_ct1 - diff_ct2);
      h_ecal_v_ecal_ctime[0]->Fill(ch2, diff_ct2 - diff_ct1);
      if (e1 > 0.5 && e2 > 0.5) {
	h_ecal_v_ecal_time[1]->Fill(ch1, diff_t1 - diff_t2);
	h_ecal_v_ecal_time[1]->Fill(ch2, diff_t2 - diff_t1);
	h_ecal_v_ecal_ctime[1]->Fill(ch1, diff_ct1 - diff_ct2);
	h_ecal_v_ecal_ctime[1]->Fill(ch2, diff_ct2 - diff_ct1);
      }
      if (0.11 <= pi0P4.M() && pi0P4.M() <= 0.16) {
	h_ecal_v_ecal_time[2]->Fill(ch1, diff_t1 - diff_t2);
	h_ecal_v_ecal_time[2]->Fill(ch2, diff_t2 - diff_t1);
	h_ecal_v_ecal_ctime[2]->Fill(ch1, diff_ct1 - diff_ct2);
	h_ecal_v_ecal_ctime[2]->Fill(ch2, diff_ct2 - diff_ct1);
	if (e1 > 0.5 && e2 > 0.5) {
	  h_ecal_v_ecal_time[3]->Fill(ch1, diff_t1 - diff_t2);
	  h_ecal_v_ecal_time[3]->Fill(ch2, diff_t2 - diff_t1);
	  h_ecal_v_ecal_ctime[3]->Fill(ch1, diff_ct1 - diff_ct2);
	  h_ecal_v_ecal_ctime[3]->Fill(ch2, diff_ct2 - diff_ct1);
	}
      }
      
      if (fabs(diff_t1) < m_TIME_CUT_RF_ECAL && fabs(diff_t2) < m_TIME_CUT_RF_ECAL) {
	
	h_ecal_cr[11]->Fill(col1 - 20, row1 - 20);
	h_ecal_cr[11]->Fill(col2 - 20, row2 - 20);
	if (0.11 <= pi0P4.M() && pi0P4.M() <= 0.16) {
	  h_ecal_cr[12]->Fill(col1 - 20, row1 - 20);
	  h_ecal_cr[12]->Fill(col2 - 20, row2 - 20);
	}
		
	for (unsigned int k = 0; k < locBeamPhotons.size(); k ++) {
	  
	  const DBeamPhoton * ebeam = locBeamPhotons[k]; 
	  double eb = ebeam->lorentzMomentum().E();
	  
	  //DetectorSystem_t sys = ebeam->dSystem;
	  //int counter = ebeam->dCounter;
	  double tb = ebeam->time();
	  double zb = ebeam->position().Z();
	  double locDeltaTRF = tb - (locRFTime + (zb - m_beamZ) / 29.9792458);
	  m_weight = 0;
	  if (fabs(locDeltaTRF) <= m_TIME_CUT_RF_TAG) {
	    m_weight = 1;
	  } else if ( ( -(m_TIME_CUT_RF_TAG + 6.0 * 2.0 * m_TIME_CUT_RF_TAG) <= locDeltaTRF && locDeltaTRF <= -(m_TIME_CUT_RF_TAG + 2.0 * m_TIME_CUT_RF_TAG) ) || 
		      ( (m_TIME_CUT_RF_TAG + 2.0 * m_TIME_CUT_RF_TAG) <= locDeltaTRF && locDeltaTRF <= (m_TIME_CUT_RF_TAG + 6.0 * 2.0 * m_TIME_CUT_RF_TAG) ) ) {
	    m_weight = -0.1;
	  } else {
	    continue;
	  }
	  h_tag_time_ecal_v_e->Fill(tb - t1 + m_time_rf_offset, eb);
	  h_tag_time_ecal_v_e->Fill(tb - t2 + m_time_rf_offset, eb);
	  
	  if (e1 > 0.5 && e2 > 0.5) {
	    h_ecal_mgg_v_blk[0]->Fill(ch1, pi0P4.M(), m_weight);
	    h_ecal_mgg_v_blk[0]->Fill(ch2, pi0P4.M(), m_weight);
	  }
	  if ((e1 > 0.5 && e2 > 0.5) && (frac1 > thres_frac1 && frac2 > thres_frac2)) {
	    h_ecal_mgg_v_blk[1]->Fill(ch1, pi0P4.M(), m_weight);
	    h_ecal_mgg_v_blk[1]->Fill(ch2, pi0P4.M(), m_weight);
	  }
	  if (tof_match1 == 0 && tof_match2 == 0) {
	    if (e1 > 0.5 && e2 > 0.5) {
	      h_ecal_mgg_v_blk[2]->Fill(ch1, pi0P4.M(), m_weight);
	      h_ecal_mgg_v_blk[2]->Fill(ch2, pi0P4.M(), m_weight);
	    }
	    if (e1 > 0.5 && e2 > 0.5 && eb > 8) {
	      h_ecal_mgg_v_blk[3]->Fill(ch1, pi0P4.M(), m_weight);
	      h_ecal_mgg_v_blk[3]->Fill(ch2, pi0P4.M(), m_weight);
	    }
	    if ((e1 > 0.5 && e2 > 0.5) && (frac1 > thres_frac1 && frac2 > thres_frac2)) {
	      h_ecal_mgg_v_blk[4]->Fill(ch1, pi0P4.M(), m_weight);
	      h_ecal_mgg_v_blk[4]->Fill(ch2, pi0P4.M(), m_weight);
	    }
	    if ((trig_bit[1] == 1 || trig_bit[3] == 1)) {
	      if (e1 > 0.5 && e2 > 0.5) {
		h_ecal_mgg_v_blk[5]->Fill(ch1, pi0P4.M(), m_weight);
		h_ecal_mgg_v_blk[5]->Fill(ch2, pi0P4.M(), m_weight);
	      }
	    }
	  }
	}
      }
    }
    for (unsigned int j = 0; j < locFCALShowers.size(); j++) {
      double e2 = locFCALShowers[j]->getEnergy();
      DVector3 position2(0, 0, 0);
      position2 = locFCALShowers[j]->getPosition() - vertex;
      double r2 = position2.Mag();
      double t2 = locFCALShowers[j]->getTime() - (r2 / TMath::C() * 1e7);
      double p2x = e2 * sin(position2.Theta()) * cos(position2.Phi());
      double p2y = e2 * sin(position2.Theta()) * sin(position2.Phi());
      double p2z = e2 * cos(position2.Theta());
      DLorentzVector photon2P4(p2x, p2y, p2z, e2);

      DVector3 position2_fdc(0, 0, 0);
      position2_fdc = locFCALShowers[j]->getPosition() - vertex_1stFDC_package;
      double p2x_fdc = e2 * sin(position2_fdc.Theta()) * cos(position2_fdc.Phi());
      double p2y_fdc = e2 * sin(position2_fdc.Theta()) * sin(position2_fdc.Phi());
      double p2z_fdc = e2 * cos(position2_fdc.Theta());
      DLorentzVector photon2P4_fdc(p2x_fdc, p2y_fdc, p2z_fdc, e2);
      
      double diff_t2 = locRFTime - t2 + m_time_rf_offset;
      const DFCALCluster *fcalCluster2;
      locFCALShowers[j]->GetSingle(fcalCluster2);
      const vector< DFCALCluster::DFCALClusterHit_t > FCALhitVector2 = fcalCluster2->GetHits();
      if (FCALhitVector2.size() < 2) continue;

      //DFCALCluster::DFCALClusterHit_t FCALhit2 = FCALhitVector2[0];
      //double ct2_max = FCALhit2.t - (r2 / TMath::C() * 1e7);
      //double diff_ct2 = locRFTime - ct2_max + m_time_rf_offset;
      
      int ch2 = fcalCluster2->getChannelEmax();
      //int col2 = fcalGeom->column(ch2);
      //int row2 = fcalGeom->row(ch2);
      //int lay2 = get_fcal_layer(row2, col2);
      double emax2 = fcalCluster2->getEmax();
      double frac2 = emax2 / fcalCluster2->getEnergy();
      DVector3  posInCal2 = fcalCluster2->getCentroid();
      float xc2 = posInCal2.Px();
      float yc2 = posInCal2.Py();
      double radiusc2 = sqrt(xc2 * xc2 + yc2 * yc2);;
      int ring2_nb = (int) (radiusc2 / (5 * k_cm));
      double thres_frac2 = 0;
      if (0 <= ring2_nb && ring2_nb <= 5) thres_frac2 = 0.5;
      if (6 <= ring2_nb && ring2_nb <= 13) thres_frac2 = 0.4;
      if (14 <= ring2_nb && ring2_nb <= 20) thres_frac2 = 0.3;
      if (21 <= ring2_nb && ring2_nb <= 25) thres_frac2 = 0.25;
      
      //double phi_fcal2 = position2.Phi();
      double delta_x_min2 = 1000.;
      double delta_y_min2 = 1000.;
      //double delta_phi_min2 = 1000.;
      //int sc_match2 = check_SC_match(phi_fcal2, locRFTime, locSCHits, delta_phi_min2);
      int tof_match2 = check_TOF_match(position2, locRFTime, vertex, locTOFPoints, delta_x_min2, delta_y_min2);
      //bool b_tof_veto2 = delta_x_min2 > 8 && delta_y_min2 > 8;
      //if (!b_tof_veto2) continue;
      //if (tof_match2 != 0) continue;
      tof_match2 = 0;
      if (delta_x_min2 < 6 && delta_y_min2 < 6) {
	tof_match2 = 1;
      }      
      DLorentzVector pi0P4_fdc = photon1P4_fdc + photon2P4_fdc;
      DLorentzVector pi0P4 = photon1P4 + photon2P4;
      if (pi0P4.M() < 0.02) continue;
      
      if (fabs(diff_t1) < m_TIME_CUT_RF_ECAL && fabs(diff_t2) < m_TIME_CUT_RF_FCAL) {
	
	for (unsigned int k = 0; k < locBeamPhotons.size(); k ++) {
	  
	  const DBeamPhoton * ebeam = locBeamPhotons[k]; 
	  //double eb = ebeam->lorentzMomentum().E();
	  
	  //DetectorSystem_t sys = ebeam->dSystem;
	  //int counter = ebeam->dCounter;
	  double tb = ebeam->time();
	  double zb = ebeam->position().Z();
	  double locDeltaTRF = tb - (locRFTime + (zb - m_beamZ) / 29.9792458);
	  m_weight = 0;
	  if (fabs(locDeltaTRF) <= m_TIME_CUT_RF_TAG) {
	    m_weight = 1;
	  } else if ( ( -(m_TIME_CUT_RF_TAG + 6.0 * 2.0 * m_TIME_CUT_RF_TAG) <= locDeltaTRF && locDeltaTRF <= -(m_TIME_CUT_RF_TAG + 2.0 * m_TIME_CUT_RF_TAG) ) || 
		      ( (m_TIME_CUT_RF_TAG + 2.0 * m_TIME_CUT_RF_TAG) <= locDeltaTRF && locDeltaTRF <= (m_TIME_CUT_RF_TAG + 6.0 * 2.0 * m_TIME_CUT_RF_TAG) ) ) {
	    m_weight = -0.1;
	  } else {
	    continue;
	  }
	  if (e1 > 0.5 && e2 > 0.5 && (locFCALShowers.size() + locECALShowers.size()) < 10) {
	    h_efcal_mgg_v_blk[0]->Fill(ch1, pi0P4.M(), m_weight);
	    h_fecal_mgg_v_blk[0]->Fill(ch2, pi0P4.M(), m_weight);
	    h_fecal_fdc_mgg_v_blk[0]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	  }
	  if (e1 > 0.5 && e2 > 0.5) {
	    h_efcal_mgg_v_blk[1]->Fill(ch1, pi0P4.M(), m_weight);
	    h_fecal_mgg_v_blk[1]->Fill(ch2, pi0P4.M(), m_weight);
	    h_fecal_fdc_mgg_v_blk[1]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	  }
	  if ((e1 > 0.5 && e2 > 0.5) && (frac1 > thres_frac1 && frac2 > thres_frac2)) {
	    h_efcal_mgg_v_blk[2]->Fill(ch1, pi0P4.M(), m_weight);
	    h_fecal_mgg_v_blk[2]->Fill(ch2, pi0P4.M(), m_weight);
	    h_fecal_fdc_mgg_v_blk[2]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	  }
	  if (tof_match1 == 0 && tof_match2 == 0) {
	    if (e1 > 0.5 && e2 > 0.5 &&  (locFCALShowers.size() + locECALShowers.size()) < 10) {
	      h_efcal_mgg_v_blk[3]->Fill(ch1, pi0P4.M(), m_weight);
	      h_fecal_mgg_v_blk[3]->Fill(ch2, pi0P4.M(), m_weight);
	      h_fecal_fdc_mgg_v_blk[3]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	    }
	    if (e1 > 0.5 && e2 > 0.5) {
	      h_efcal_mgg_v_blk[4]->Fill(ch1, pi0P4.M(), m_weight);
	      h_fecal_mgg_v_blk[4]->Fill(ch2, pi0P4.M(), m_weight);
	      h_fecal_fdc_mgg_v_blk[4]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	    }
	    if ((e1 > 0.5 && e2 > 0.5) && (frac1 > thres_frac1 && frac2 > thres_frac2)) {
	      h_efcal_mgg_v_blk[5]->Fill(ch1, pi0P4.M(), m_weight);
	      h_fecal_mgg_v_blk[5]->Fill(ch2, pi0P4.M(), m_weight);
	      h_fecal_fdc_mgg_v_blk[5]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	    }
	    if ((trig_bit[1] == 1 || trig_bit[3] == 1)) {
	      if (e1 > 0.5 && e2 > 0.5 &&  (locFCALShowers.size() + locECALShowers.size()) < 10) {
		h_efcal_mgg_v_blk[6]->Fill(ch1, pi0P4.M(), m_weight);
		h_fecal_mgg_v_blk[6]->Fill(ch2, pi0P4.M(), m_weight);
		h_fecal_fdc_mgg_v_blk[6]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	      }
	    }
	  }
	}
      }
    }
  }

  for (unsigned int i = 0; i < locFCALShowers.size(); i++) {
    double e1 = locFCALShowers[i]->getEnergy();
    DVector3 position1(0, 0, 0);
    position1 = locFCALShowers[i]->getPosition() - vertex;
    double r1 = position1.Mag();
    double t1 = locFCALShowers[i]->getTime() - (r1 / TMath::C() * 1e7);
    double p1x = e1 * sin(position1.Theta()) * cos(position1.Phi());
    double p1y = e1 * sin(position1.Theta()) * sin(position1.Phi());
    double p1z = e1 * cos(position1.Theta());
    DLorentzVector photon1P4(p1x, p1y, p1z, e1);

    DVector3 position1_fdc(0, 0, 0);
    position1_fdc = locFCALShowers[i]->getPosition() - vertex_1stFDC_package;
    double p1x_fdc = e1 * sin(position1_fdc.Theta()) * cos(position1_fdc.Phi());
    double p1y_fdc = e1 * sin(position1_fdc.Theta()) * sin(position1_fdc.Phi());
    double p1z_fdc = e1 * cos(position1_fdc.Theta());
    DLorentzVector photon1P4_fdc(p1x_fdc, p1y_fdc, p1z_fdc, e1);
    
    double diff_t1 = locRFTime - t1 + m_time_rf_offset;;
    const DFCALCluster *fcalCluster1;
    locFCALShowers[i]->GetSingle(fcalCluster1);
    const vector< DFCALCluster::DFCALClusterHit_t > FCALhitVector1 = fcalCluster1->GetHits();
    if (FCALhitVector1.size() < 2) continue;
    DFCALCluster::DFCALClusterHit_t FCALhit1 = FCALhitVector1[0];
    double ct1_max = FCALhit1.t - (r1 / TMath::C() * 1e7);
    double diff_ct1 = locRFTime - ct1_max + m_time_rf_offset;
    int ch1 = FCALhit1.ch;
    double cpx1 = FCALhit1.x - vertex.X() + m_fcalX;
    double cpy1 = FCALhit1.y - vertex.Y() + m_fcalY;
    double cpz1 = position1.Z();
    double cr1 = sqrt(pow(cpx1, 2) + pow(cpy1, 2) + pow(cpz1, 2));
    double t1_corr = (m_fcalZ + DFCALGeometry::blockLength() - locFCALShowers[i]->getPosition().Z()) / FCAL_C_EFFECTIVE;
    diff_ct1 = locRFTime - (FCALhit1.t - t1_corr - (cr1 / c));
    
    int col1 = fcalGeom->column(ch1);
    int row1 = fcalGeom->row(ch1);
    //int lay1 = get_fcal_layer(row1, col1);
    //h_fcal_cluster_size->Fill(FCALhitVector1.size());
    //h_fcal_layer->Fill(lay1);
    
    //cout <<"chi1 " << ch1 << " col1 " << col1 << " row1 " << row1 << endl;
    double emax1 = fcalCluster1->getEmax();
    double ce1 = fcalCluster1->getEnergy();
    double frac1 = emax1 / fcalCluster1->getEnergy();
    DVector3  posInCal1 = fcalCluster1->getCentroid();
    float xc1 = posInCal1.Px();
    float yc1 = posInCal1.Py();
    double radiusc1 = sqrt(xc1 * xc1 + yc1 * yc1);;
    int ring1_nb = (int) (radiusc1 / (5 * k_cm));
    double thres_frac1 = 0;
    if (0 <= ring1_nb && ring1_nb <= 5) thres_frac1 = 0.5;
    if (6 <= ring1_nb && ring1_nb <= 13) thres_frac1 = 0.4;
    if (14 <= ring1_nb && ring1_nb <= 20) thres_frac1 = 0.3;
    if (21 <= ring1_nb && ring1_nb <= 25) thres_frac1 = 0.25;
    h_fcal_e_v_ce->Fill(ce1, ce1 / e1);
    h_fcal_e_ratio->Fill(ce1, emax1 / ce1);    
    /*
    int ChannelNumber =  fcalHit.ch;
    double chanx      =  fcalHit.x       - vertex.X()  +  fcalX;
    double chany      =  fcalHit.y       - vertex.Y()  +  fcalY;
    double chanz      =  pos_corrected_z - vertex.Z();
    double hitTime    =  fcalHit.t;
    double dR = sqrt( chanx*chanx  +  chany*chany  +  chanz*chanz );
    // propagate hit time to the interaction vertex:                                                                                                                                                                                  
    double tCorr   = ( fcalZ + DFCALGeometry::blockLength() - pos_corrected_z )
      / FCAL_C_EFFECTIVE;
    hitTime = hitTime  -  tCorr  -  (dR/c);
    double fcal_rf_dt = hitTime - rfTime;
    */


    //h_fcal_blk_v_layer->Fill(lay1 - 10, ch1);
    //h_fcal_ring->Fill(ring1_nb);
    //h_fcal_cr_ring[lay1 - laymin]->Fill(col1 - 29, row1 - 29);
    //double phi_fcal1 = position1.Phi();
    double delta_x_min1 = 1000.;
    double delta_y_min1 = 1000.;
    //double delta_phi_min1 = 1000.;
    //int sc_match1 = check_SC_match(phi_fcal1, locRFTime, locSCHits, delta_phi_min1);
    int tof_match1 = check_TOF_match(position1, locRFTime, vertex, locTOFPoints, delta_x_min1, delta_y_min1);
    //bool b_tof_veto1 = delta_x_min1 > 8 && delta_y_min1 > 8; 
    //if (!b_tof_veto1) continue;
    //if (tof_match1 != 0) continue;
    tof_match1 = 0;
    if (delta_x_min1 < 6 && delta_y_min1 < 6) {
      tof_match1 = 1;
    }

    if (tof_match1 == 0 && fabs(diff_t1) < 4.005) {
      Photons_fcal_list.push_back(locFCALShowers[i]);
      photons_fcal_list.push_back(photon1P4);
    }
    
    //int tofs_match1 = 0;
    //if (delta_x_min1 < 10 && delta_y_min1 < 10) {
    //tofs_match1 = 1;
    //}      
    
    h_fcal_xy[0]->Fill(xc1, yc1);
    h_fcal_cr[0]->Fill(col1 - 29, row1 - 29);
    for (int k = 1; k < 11; k ++) {
      if (emax1 >= ((double) k)) {
	h_fcal_xy[k]->Fill(xc1, yc1);
	h_fcal_cr[k]->Fill(col1 - 29, row1 - 29);
      }
    }
    h_fcal_time[0]->Fill(ch1, diff_t1);
    h_fcal_ctime[0]->Fill(ch1, diff_ct1);
    if (trig_bit[0] == 1) h_fcal_time_v_bit[0]->Fill(ch1, diff_t1);
    if (trig_bit[1] == 1 && trig_bit[3] == 0) h_fcal_time_v_bit[1]->Fill(ch1, diff_t1);
    if (trig_bit[2] == 1) h_fcal_time_v_bit[2]->Fill(ch1, diff_t1);
    if (trig_bit[3] == 1 && trig_bit[1] == 0) h_fcal_time_v_bit[3]->Fill(ch1, diff_t1);
    if (trig_bit[4] == 1) h_fcal_time_v_bit[4]->Fill(ch1, diff_t1);
    if (trig_bit[1] == 1 && trig_bit[3] == 1) h_fcal_time_v_bit[5]->Fill(ch1, diff_t1);
    if (trig_bit[0] == 1) h_fcal_ctime_v_bit[0]->Fill(ch1, diff_ct1);
    if (trig_bit[1] == 1 && trig_bit[3] == 0) h_fcal_ctime_v_bit[1]->Fill(ch1, diff_ct1);
    if (trig_bit[2] == 1) h_fcal_ctime_v_bit[2]->Fill(ch1, diff_ct1);
    if (trig_bit[3] == 1 && trig_bit[1] == 0) h_fcal_ctime_v_bit[3]->Fill(ch1, diff_ct1);
    if (trig_bit[4] == 1) h_fcal_ctime_v_bit[4]->Fill(ch1, diff_ct1);
    if (trig_bit[1] == 1 && trig_bit[3] == 1) h_fcal_ctime_v_bit[5]->Fill(ch1, diff_ct1);

    for (unsigned int j = i + 1; j < locFCALShowers.size(); j++) {
      double e2 = locFCALShowers[j]->getEnergy();
      DVector3 position2(0, 0, 0);
      position2 = locFCALShowers[j]->getPosition() - vertex;
      double r2 = position2.Mag();
      double t2 = locFCALShowers[j]->getTime() - (r2 / TMath::C() * 1e7);
      double p2x = e2 * sin(position2.Theta()) * cos(position2.Phi());
      double p2y = e2 * sin(position2.Theta()) * sin(position2.Phi());
      double p2z = e2 * cos(position2.Theta());
      DLorentzVector photon2P4(p2x, p2y, p2z, e2);

      DVector3 position2_fdc(0, 0, 0);
      position2_fdc = locFCALShowers[j]->getPosition() - vertex_1stFDC_package;
      double p2x_fdc = e2 * sin(position2_fdc.Theta()) * cos(position2_fdc.Phi());
      double p2y_fdc = e2 * sin(position2_fdc.Theta()) * sin(position2_fdc.Phi());
      double p2z_fdc = e2 * cos(position2_fdc.Theta());
      DLorentzVector photon2P4_fdc(p2x_fdc, p2y_fdc, p2z_fdc, e2);
      
      double diff_t2 = locRFTime - t2 + m_time_rf_offset;;
      const DFCALCluster *fcalCluster2;
      locFCALShowers[j]->GetSingle(fcalCluster2);
      const vector< DFCALCluster::DFCALClusterHit_t > FCALhitVector2 = fcalCluster2->GetHits();
      if (FCALhitVector2.size() < 2) continue;
      DFCALCluster::DFCALClusterHit_t FCALhit2 = FCALhitVector2[0];
      double ct2_max = FCALhit2.t - (r2 / TMath::C() * 1e7);
      double diff_ct2 = locRFTime - ct2_max + m_time_rf_offset;
      //int ch2 = fcalCluster2->getChannelEmax();
      int ch2 = FCALhit2.ch;
      double cpx2 = FCALhit2.x - vertex.X() + m_fcalX;
      double cpy2 = FCALhit2.y - vertex.Y() + m_fcalY;
      double cpz2 = position2.Z();
      double cr2 = sqrt(pow(cpx2, 2) + pow(cpy2, 2) + pow(cpz2, 2));
      double t2_corr = (m_fcalZ + DFCALGeometry::blockLength() - locFCALShowers[i]->getPosition().Z()) / FCAL_C_EFFECTIVE;
      diff_ct2 = locRFTime - (FCALhit2.t - t2_corr - (cr2 / c));
      int col2 = fcalGeom->column(ch2);
      int row2 = fcalGeom->row(ch2);
      //int lay2 = get_fcal_layer(row2, col2);
      double emax2 = fcalCluster2->getEmax();
      double frac2 = emax2 / fcalCluster2->getEnergy();
      DVector3  posInCal2 = fcalCluster2->getCentroid();
      float xc2 = posInCal2.Px();
      float yc2 = posInCal2.Py();
      double radiusc2 = sqrt(xc2 * xc2 + yc2 * yc2);;
      int ring2_nb = (int) (radiusc2 / (4 * k_cm));
      double thres_frac2 = 0;
      if (0 <= ring2_nb && ring2_nb <= 5) thres_frac2 = 0.5;
      if (6 <= ring2_nb && ring2_nb <= 13) thres_frac2 = 0.4;
      if (14 <= ring2_nb && ring2_nb <= 20) thres_frac2 = 0.3;
      if (21 <= ring2_nb && ring2_nb <= 25) thres_frac2 = 0.25;
    
      //double phi_fcal2 = position2.Phi();
      double delta_x_min2 = 1000.;
      double delta_y_min2 = 1000.;
      //double delta_phi_min2 = 1000.;
      //int sc_match2 = check_SC_match(phi_fcal2, locRFTime, locSCHits, delta_phi_min2);
      int tof_match2 = check_TOF_match(position2, locRFTime, vertex, locTOFPoints, delta_x_min2, delta_y_min2);
      //bool b_tof_veto2 = delta_x_min2 > 8 && delta_y_min2 > 8;
      //if (!b_tof_veto2) continue;
      //if (tof_match2 != 0) continue;
      tof_match2 = 0;
      if (delta_x_min2 < 6 && delta_y_min2 < 6) {
	tof_match2 = 1;
      }
      //int tofs_match2 = 0;
      //if (delta_x_min2 < 10 && delta_y_min2 < 10) {
      //tofs_match2 = 1;
      //}
      
      DLorentzVector pi0P4 = photon1P4 + photon2P4;
      DLorentzVector pi0P4_fdc = photon1P4_fdc + photon2P4_fdc;

      if (pi0P4.M() < 0.02) continue;

      h_fcal_time[1]->Fill(ch1, diff_t1);
      h_fcal_time[1]->Fill(ch2, diff_t2);
      h_fcal_ctime[1]->Fill(ch1, diff_ct1);
      h_fcal_ctime[1]->Fill(ch2, diff_ct2);

      if (e1 > 0.5 && e2 > 0.5 && locFCALShowers.size() < 10) {
	h_fcal_time[2]->Fill(ch1, diff_t1);
	h_fcal_time[2]->Fill(ch2, diff_t2);
	h_fcal_ctime[2]->Fill(ch1, diff_ct1);
	h_fcal_ctime[2]->Fill(ch2, diff_ct2);
	if (tof_match1 == 0 && tof_match2 == 0) {
	  h_fcal_time[3]->Fill(ch1, diff_t1);
	  h_fcal_time[3]->Fill(ch2, diff_t2);
	  h_fcal_ctime[3]->Fill(ch1, diff_ct1);
	  h_fcal_ctime[3]->Fill(ch2, diff_ct2);
	  if (0.11 <= pi0P4.M() && pi0P4.M() <= 0.16) {
	    h_fcal_time[4]->Fill(ch1, diff_t1);
	    h_fcal_time[4]->Fill(ch2, diff_t2);
	    h_fcal_ctime[4]->Fill(ch1, diff_ct1);
	    h_fcal_ctime[4]->Fill(ch2, diff_ct2);
	  }
	}
      }
      h_fcal_v_fcal_time[0]->Fill(ch1, diff_t1 - diff_t2);
      h_fcal_v_fcal_time[0]->Fill(ch2, diff_t2 - diff_t1);
      h_fcal_v_fcal_ctime[0]->Fill(ch1, diff_ct1 - diff_ct2);
      h_fcal_v_fcal_ctime[0]->Fill(ch2, diff_ct2 - diff_ct1);
      if (e1 > 0.5 && e2 > 0.5) {
	h_fcal_v_fcal_time[1]->Fill(ch1, diff_t1 - diff_t2);
	h_fcal_v_fcal_time[1]->Fill(ch2, diff_t2 - diff_t1);
	h_fcal_v_fcal_ctime[1]->Fill(ch1, diff_ct1 - diff_ct2);
	h_fcal_v_fcal_ctime[1]->Fill(ch2, diff_ct2 - diff_ct1);
      }
      if (0.11 <= pi0P4.M() && pi0P4.M() <= 0.16) {
	h_fcal_v_fcal_time[2]->Fill(ch1, diff_t1 - diff_t2);
	h_fcal_v_fcal_time[2]->Fill(ch2, diff_t2 - diff_t1);
	h_fcal_v_fcal_ctime[2]->Fill(ch1, diff_ct1 - diff_ct2);
	h_fcal_v_fcal_ctime[2]->Fill(ch2, diff_ct2 - diff_ct1);
	if (e1 > 0.5 && e2 > 0.5) {
	  h_fcal_v_fcal_time[3]->Fill(ch1, diff_t1 - diff_t2);
	  h_fcal_v_fcal_time[3]->Fill(ch2, diff_t2 - diff_t1);
	  h_fcal_v_fcal_ctime[3]->Fill(ch1, diff_ct1 - diff_ct2);
	  h_fcal_v_fcal_ctime[3]->Fill(ch2, diff_ct2 - diff_ct1);
	}
      }
      
      if (fabs(diff_t1) < m_TIME_CUT_RF_FCAL && fabs(diff_t2) < m_TIME_CUT_RF_FCAL) {
	
	h_fcal_cr[11]->Fill(col1 - 29, row1 - 29);
	h_fcal_cr[11]->Fill(col2 - 29, row2 - 29);
	
	if (0.11 <= pi0P4.M() && pi0P4.M() <= 0.16) {
	  h_fcal_cr[12]->Fill(col1 - 29, row1 - 29);
	  h_fcal_cr[12]->Fill(col2 - 29, row2 - 29);
	}
	
	for (unsigned int k = 0; k < locBeamPhotons.size(); k ++) {
	  
	  const DBeamPhoton * ebeam = locBeamPhotons[k]; 
	  double eb = ebeam->lorentzMomentum().E();
	  	  
	  //DetectorSystem_t sys = ebeam->dSystem;
	  //int counter = ebeam->dCounter;
	  double tb = ebeam->time();
	  double zb = ebeam->position().Z();
	  double locDeltaTRF = tb - (locRFTime + (zb - m_beamZ) / 29.9792458);
	  h_tag_time_rf_v_e->Fill(locDeltaTRF, eb);
	  h_tag_time_fcal_v_e->Fill(tb - t1 + m_time_rf_offset, eb);
	  h_tag_time_fcal_v_e->Fill(tb - t2 + m_time_rf_offset, eb);
	  m_weight = 0;
	  if (fabs(locDeltaTRF) <= m_TIME_CUT_RF_TAG) {
	    m_weight = 1;
	  } else if ( ( -(m_TIME_CUT_RF_TAG + 6.0 * 2.0 * m_TIME_CUT_RF_TAG) <= locDeltaTRF && locDeltaTRF <= -(m_TIME_CUT_RF_TAG + 2.0 * m_TIME_CUT_RF_TAG) ) || 
		      ( (m_TIME_CUT_RF_TAG + 2.0 * m_TIME_CUT_RF_TAG) <= locDeltaTRF && locDeltaTRF <= (m_TIME_CUT_RF_TAG + 6.0 * 2.0 * m_TIME_CUT_RF_TAG) ) ) {
	    m_weight = -0.1;
	  } else {
	    continue;
	  }

	  if ((e1 > 0.5 && e2 > 0.5) && (frac1 > thres_frac1 && frac2 > thres_frac2)) {
	    h_fcal_mgg_v_blk[0]->Fill(ch1, pi0P4.M(), m_weight);
	    h_fcal_mgg_v_blk[0]->Fill(ch2, pi0P4.M(), m_weight);
	    h_fcal_fdc_mgg_v_blk[0]->Fill(ch1, pi0P4_fdc.M(), m_weight);
	    h_fcal_fdc_mgg_v_blk[0]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	  }
	  if (tof_match1 == 0 && tof_match2 == 0) {
	    
	    if (e1 > 0.5 && e2 > 0.5 && locFCALShowers.size() < 10) {
	      h_fcal_mgg_v_blk[1]->Fill(ch1, pi0P4.M(), m_weight);
	      h_fcal_mgg_v_blk[1]->Fill(ch2, pi0P4.M(), m_weight);
	      h_fcal_fdc_mgg_v_blk[1]->Fill(ch1, pi0P4_fdc.M(), m_weight);
	      h_fcal_fdc_mgg_v_blk[1]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	    }
	    if (e1 > 0.5 && e2 > 0.5) {
	      h_fcal_mgg_v_blk[2]->Fill(ch1, pi0P4.M(), m_weight);
	      h_fcal_mgg_v_blk[2]->Fill(ch2, pi0P4.M(), m_weight);
	      h_fcal_fdc_mgg_v_blk[2]->Fill(ch1, pi0P4_fdc.M(), m_weight);
	      h_fcal_fdc_mgg_v_blk[2]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	    }
	    
	    if (e1 > 0.5 && e2 > 0.5 && locFCALShowers.size() < 10) {
	      h_fcal_mgg_v_blk[3]->Fill(ch1, pi0P4.M(), m_weight);
	      h_fcal_mgg_v_blk[3]->Fill(ch2, pi0P4.M(), m_weight);
	      h_fcal_fdc_mgg_v_blk[3]->Fill(ch1, pi0P4_fdc.M(), m_weight);
	      h_fcal_fdc_mgg_v_blk[3]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	    }
	    if (e1 > 0.5 && e2 > 0.5 && eb > 8) {
	      h_fcal_mgg_v_blk[4]->Fill(ch1, pi0P4.M(), m_weight);
	      h_fcal_mgg_v_blk[4]->Fill(ch2, pi0P4.M(), m_weight);
	      h_fcal_fdc_mgg_v_blk[4]->Fill(ch1, pi0P4_fdc.M(), m_weight);
	      h_fcal_fdc_mgg_v_blk[4]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	    }
	    if ((e1 > 0.5 && e2 > 0.5) && (frac1 > thres_frac1 && frac2 > thres_frac2)) {
	      h_fcal_mgg_v_blk[5]->Fill(ch1, pi0P4.M(), m_weight);
	      h_fcal_mgg_v_blk[5]->Fill(ch2, pi0P4.M(), m_weight);
	      h_fcal_fdc_mgg_v_blk[5]->Fill(ch1, pi0P4_fdc.M(), m_weight);
	      h_fcal_fdc_mgg_v_blk[5]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	    }
	    if ((trig_bit[1] == 1 || trig_bit[3] == 1)) {
	      if (e1 > 0.5 && e2 > 0.5 && locFCALShowers.size() < 10) {
		h_fcal_mgg_v_blk[6]->Fill(ch1, pi0P4.M(), m_weight);
		h_fcal_mgg_v_blk[6]->Fill(ch2, pi0P4.M(), m_weight);
		h_fcal_fdc_mgg_v_blk[6]->Fill(ch1, pi0P4_fdc.M(), m_weight);
		h_fcal_fdc_mgg_v_blk[6]->Fill(ch2, pi0P4_fdc.M(), m_weight);
	      }
	    }
	  }
	}
      }
    }
  }

  if (Photons_ecal_list.size() == 2) {
    TLorentzVector wP4 = photons_ecal_list[0] + photons_ecal_list[1];
     h_ecal_im[4]->Fill(wP4.M());
  }
  if (Photons_fcal_list.size() == 2) {
    TLorentzVector wP4 = photons_fcal_list[0] + photons_fcal_list[1];
     h_fcal_im[4]->Fill(wP4.M());
  }
  
  if (Photons_ecal_list.size() == 3) {
    TLorentzVector wP4 = photons_ecal_list[0] + photons_ecal_list[1] + photons_ecal_list[2];
    TLorentzVector wggP4[3];
    TLorentzVector wgP4[3];
    wggP4[0] = photons_ecal_list[0] + photons_ecal_list[1];
    wggP4[1] = photons_ecal_list[0] + photons_ecal_list[2];
    wggP4[2] = photons_ecal_list[1] + photons_ecal_list[2];
    wgP4[0] = photons_ecal_list[2];
    wgP4[1] = photons_ecal_list[1];
    wgP4[2] = photons_ecal_list[0];
    h_ecal_im[0]->Fill(wP4.M());
    double best_chi2 = 1e7;
    int ibest = -1;
    for (int i = 0; i < 3; i ++) {
      double chi2 = TMath::Power((wggP4[i].M() - m_pi0) / 8.0e-3,2.0);
      if (chi2 < best_chi2) {
	chi2 = best_chi2;
	ibest = i;
      }
    }
    /*
    int word[3];
    if (ibest == 0) {
      word[0] = 0;
      word[1] = 1;
      word[2] = 2;
    } else if (ibest == 1) {
      word[0] = 0;
      word[1] = 2;
      word[2] = 1;
    } else if (ibest == 2) {
      word[0] = 1;
      word[1] = 2;
      word[2] = 0;
    } 
    */
    if (ibest >= 0) {
      h_ecal_im[1]->Fill(wggP4[ibest].M());
      h_ecal_im[2]->Fill(wP4.M());
      if (0.67 < wP4.M() && wP4.M() < 0.85) {
	h_ecal_im[3]->Fill(wggP4[ibest].M());
	double ecl_tab[3] = {0, 0, 0};
	for (int i = 0; i < 3; i ++) {
	  const DECALCluster * ecalCluster;
	  Photons_ecal_list[i]->GetSingle(ecalCluster);
	  double cl_e = ecalCluster->E;
	  ecl_tab[i] = cl_e;
	}
	bool sym_cl = 0.85 < ecl_tab[0] / ecl_tab[1] && ecl_tab[0] / ecl_tab[1] < 1.15;
	for (int i = 0; i < 3; i ++) {
	  //double e = Photons_ecal_list[i]->E;
	  DVector3 position = Photons_ecal_list[i]->pos - vertex;
	  double r = position.Mag();
	  double t = Photons_ecal_list[i]->t - (r / TMath::C() * 1e7);
	  double diff_t = t - locRFTime;
	  double face_x = vertex.X() + (position.X() * (m_ecalZ - vertex.Z())/position.Z());
	  double face_y = vertex.Y() + (position.Y() * (m_ecalZ - vertex.Z())/position.Z());
	  double ra = sqrt(pow(face_x, 2) + pow(face_y, 2));
	  const DECALCluster *ecalCluster;
	  Photons_ecal_list[i]->GetSingle(ecalCluster);
	  double cl_e = ecalCluster->E;
	  int rg = (int) (ra / (2.5 * k_cm));
	  if (fabs(diff_t) < 3.005) {
	    h_ecal_nl[rg]->Fill(cl_e, wggP4[ibest].M());
	    //h_ecal_cs->Fill(cl_e / e);    
	      
	    h_ecal_wnl[i][rg]->Fill(cl_e, wggP4[ibest].M());
	    //h_ecal_wcs[i]->Fill(cl_e / e);    
	    if (sym_cl) {
	      if (i < 2) h_ecal_snl[i][rg]->Fill(cl_e, wggP4[ibest].M());
	    }
	    if (i == 2) h_ecal_snl[i][rg]->Fill(cl_e, wggP4[ibest].M());
	  }
	}
      }
    }
  }
  
  if (Photons_fcal_list.size() == 3) {
    TLorentzVector wP4 = photons_fcal_list[0] + photons_fcal_list[1] + photons_fcal_list[2];
    TLorentzVector wggP4[3];
    TLorentzVector wgP4[3];
    wggP4[0] = photons_fcal_list[0] + photons_fcal_list[1];
    wggP4[1] = photons_fcal_list[0] + photons_fcal_list[2];
    wggP4[2] = photons_fcal_list[1] + photons_fcal_list[2];
    wgP4[0] = photons_fcal_list[2];
    wgP4[1] = photons_fcal_list[1];
    wgP4[2] = photons_fcal_list[0];
    h_fcal_im[0]->Fill(wP4.M());
    double best_chi2 = 1e7;
    int ibest = -1;
    for (int i = 0; i < 3; i ++) {
      double chi2 = TMath::Power((wggP4[i].M() - m_pi0) / 8.0e-3,2.0);
      if (chi2 < best_chi2 /*&& 0.11 < wggP4[i].M() && wggP4[i].M() < 0.16*/) {
	chi2 = best_chi2;
	ibest = i;
      }
    }
    /*
    int word[3];
    if (ibest == 0) {
      word[0] = 0;
      word[1] = 1;
      word[2] = 2;
    } else if (ibest == 1) {
      word[0] = 0;
      word[1] = 2;
      word[2] = 1;
    } else if (ibest == 2) {
      word[0] = 1;
      word[1] = 2;
      word[2] = 0;
    } 
    */
    if (ibest >= 0) {
      h_fcal_im[1]->Fill(wggP4[ibest].M());
      h_fcal_im[2]->Fill(wP4.M());
      if (0.67 < wP4.M() && wP4.M() < 0.85) {
	h_fcal_im[3]->Fill(wggP4[ibest].M());
	double ecl_tab[3] = {0, 0, 0};
	for (int i = 0; i < 3; i ++) {
	  const DFCALCluster * fcalCluster;
	  Photons_fcal_list[i]->GetSingle(fcalCluster);
	  double cl_e = fcalCluster->getEnergy();
	  ecl_tab[i] = cl_e;
	}
	bool sym_cl = 0.85 < ecl_tab[0] / ecl_tab[1] && ecl_tab[0] / ecl_tab[1] < 1.15;
	for (int i = 0; i < 3; i ++) {
	  //double e = Photons_fcal_list[i]->getEnergy();
	  DVector3 position = Photons_fcal_list[i]->getPosition() - vertex;
	  double r = position.Mag();
	  double t = Photons_fcal_list[i]->getTime() - (r / TMath::C() * 1e7);
	  double diff_t = t - locRFTime;
	  double face_x = vertex.X() + (position.X() * (m_fcalZ - vertex.Z())/position.Z());
	  double face_y = vertex.Y() + (position.Y() * (m_fcalZ - vertex.Z())/position.Z());
	  double ra = sqrt(pow(face_x, 2) + pow(face_y, 2));
	  const DFCALCluster *fcalCluster;
	  Photons_fcal_list[i]->GetSingle(fcalCluster);
	  double cl_e = fcalCluster->getEnergy();
	  int rg = (int) (ra / (5 * k_cm));
	  if (fabs(diff_t) < 3.005) {
	    h_fcal_nl[rg]->Fill(cl_e, wggP4[ibest].M());
	    //h_fcal_cs->Fill(cl_e / e);    
	      
	    h_fcal_wnl[i][rg]->Fill(cl_e, wggP4[ibest].M());
	    //h_fcal_wcs[i]->Fill(cl_e / e);    
	    if (sym_cl) {
	      if (i < 2) h_fcal_snl[i][rg]->Fill(cl_e, wggP4[ibest].M());
	    }
	    if (i == 2) h_fcal_snl[i][rg]->Fill(cl_e, wggP4[ibest].M());
	  }
	}
      }
    }
  }
  
  lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK  

  

  
  return;
}

//------------------
// erun
//------------------
void JEventProcessor_cal_cal::EndRun()
{
	
	return;
}

//------------------
// fini
//------------------
void JEventProcessor_cal_cal::Finish()
{
  return;
}

int JEventProcessor_cal_cal::check_TOF_match(DVector3 fcalpos, double rfTime, DVector3 vertex, vector< const DTOFPoint* > locTOFPoints, double &dx_min, double &dy_min) 
{
  int global_tof_match = 0;
  dx_min = 1000;
  dy_min = 1000;
  for (vector< const DTOFPoint* >::const_iterator tof = locTOFPoints.begin(); tof != locTOFPoints.end(); tof++) {
    
    double xt = (*tof)->pos.X() - vertex.X();
    double yt = (*tof)->pos.Y() - vertex.Y();
    double zt = (*tof)->pos.Z() - vertex.Z();
    double rt = sqrt(xt*xt + yt*yt + zt*zt);
    double tt = (*tof)->t - (rt / TMath::C() * 1e7);
    double dt = tt - rfTime;
    h_tof_time->Fill(dt);
    xt *= fcalpos.Z() / zt;
    yt *= fcalpos.Z() / zt;
    double dx = fcalpos.X() - xt;
    double dy = fcalpos.Y() - yt;
    if (fabs(dt) < m_TIME_CUT_RF_TOF) {
      if (dx*dx + dy*dy < dx_min*dx_min + dy_min*dy_min) {
	dx_min = fabs(dx);
	dy_min = fabs(dy);
      }
      global_tof_match ++;
    }
  }
  
  return global_tof_match;
}

int JEventProcessor_cal_cal::check_SC_match(double phi, double rfTime, vector< const DSCHit* > locSCHits, double &dphi_min) 
{
  int global_sc_match = 0;
  dphi_min = 1000.;
  for (unsigned int i = 0; i < locSCHits.size(); i ++) {
    
    const DSCHit *schits = locSCHits[i];
    double t = schits->t;
    double e = schits->dE;
    double s = schits->sector - 1;
    double diff_t = t - rfTime;
    h_sc_time_v_e->Fill(diff_t, e);
    double phi_sc = sc_pos[s][0].Phi();
    double dphi = phi - phi_sc;
    if (dphi < -TMath::Pi()) 
      dphi += 2.0 * TMath::Pi();
    if (dphi > TMath::Pi()) 
      dphi -= 2.0 * TMath::Pi();
    if (fabs(dphi) < dphi_min)
      dphi_min = fabs(dphi);
    if ((m_TIME_CUT_RF_SC_MIN < diff_t) && (diff_t < m_TIME_CUT_RF_SC_MAX) && ((e * 1e3) > m_ENERGY_CUT_SC)) {
      global_sc_match ++;
    }
  } 
  
  return global_sc_match;
}

int JEventProcessor_cal_cal::get_ecal_layer(int row, int col)
{
  int m_row = row > 19 ? (row - 20) : (19 - row);
  int m_col = col > 19 ? (col - 20) : (19 - col);
  int layer = m_row > m_col ? m_row : m_col;
  return layer;
}

int JEventProcessor_cal_cal::get_fcal_layer(int row, int col)
{
  int m_row = row > 30 ? (row - 29) : (29 - row);
  int m_col = col > 30 ? (col - 29) : (29 - col);
  int layer = m_row > m_col ? m_row : m_col;
  return layer;
}
