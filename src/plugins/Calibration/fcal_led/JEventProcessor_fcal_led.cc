// $Id$
//
//    File: JEventProcessor_fcal_led.cc
// Created: Fri May 19 12:39:24 EDT 2017
// Creator: mashephe (on Linux stanley.physics.indiana.edu 2.6.32-642.6.2.el6.x86_64 unknown)
//

#include "JEventProcessor_fcal_led.h"
#include <DANA/DEvent.h>

extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_fcal_led());
  }
} // "C"

//define static local variable //declared in header file
thread_local DTreeFillData JEventProcessor_fcal_led::dTreeFill;

//------------------
// JEventProcessor_fcal_led (Constructor)
//------------------
void JEventProcessor_fcal_led::Init()
{
  auto app = GetApplication();
  m_LED_min1 = 7.16453;
  m_LED_min2 = 13.211;
  m_LED_min3 = 16.1517;
  m_LED_min4 = 24.25;
  m_LED_min5 = 30.186;
  m_LED_max1 = 9.76453;
  m_LED_max2 = 15.811;
  m_LED_max3 = 18.7517;
  m_LED_max4 = 26.85;
  m_LED_max5 = 32.786;
  
  m_do_tree = false;
  app->SetDefaultParameter( "FCAL-LED:do_tree", m_do_tree);
  m_is_fcal_led_skim = true;
  app->SetDefaultParameter( "FCAL-LED:is_rawdata", m_is_fcal_led_skim);

  app->SetDefaultParameter( "FCAL-LED:LED_min1", m_LED_min1);
  app->SetDefaultParameter( "FCAL-LED:LED_min2", m_LED_min2);
  app->SetDefaultParameter( "FCAL-LED:LED_min3", m_LED_min3);
  app->SetDefaultParameter( "FCAL-LED:LED_min4", m_LED_min4);
  app->SetDefaultParameter( "FCAL-LED:LED_min5", m_LED_min5);
  app->SetDefaultParameter( "FCAL-LED:LED_min6", m_LED_min6);

  app->SetDefaultParameter( "FCAL-LED:LED_max1", m_LED_max1);
  app->SetDefaultParameter( "FCAL-LED:LED_max2", m_LED_max2);
  app->SetDefaultParameter( "FCAL-LED:LED_max3", m_LED_max3);
  app->SetDefaultParameter( "FCAL-LED:LED_max4", m_LED_max4);
  app->SetDefaultParameter( "FCAL-LED:LED_max5", m_LED_max5);
  app->SetDefaultParameter( "FCAL-LED:LED_max6", m_LED_max6);
  m_regions_inf[0] = m_LED_min1;
  m_regions_sup[0] = m_LED_max1;
  m_regions_inf[1] = m_LED_min2;
  m_regions_sup[1] = m_LED_max2;
  m_regions_inf[2] = m_LED_min3;
  m_regions_sup[2] = m_LED_max3;
  m_regions_inf[3] = m_LED_min4;
  m_regions_sup[3] = m_LED_max4;
  m_regions_inf[4] = m_LED_min5;
  m_regions_sup[4] = m_LED_max5;
  m_regions_inf[5] = m_LED_min6;
  m_regions_sup[5] = m_LED_max6;
  for (int i = 0; i < 5; i ++) {
    cout << "inf " << m_regions_inf[i] << " sup " << m_regions_sup[i] << endl;
    m_regions_inf[i] *= 1e6;
    m_regions_sup[i] *= 1e6;
  }
  dTreeInterface = DTreeInterface::Create_DTreeInterface("led", "tree_fcal_led.root");
  DTreeBranchRegister locTreeBranchRegister;
  if (m_do_tree) {
    locTreeBranchRegister.Register_Single<Int_t>("rnb");
    locTreeBranchRegister.Register_Single<Int_t>("evt");
    locTreeBranchRegister.Register_Single<Float_t>("eTot");
    locTreeBranchRegister.Register_Single<Int_t>("nHits");
    locTreeBranchRegister.Register_FundamentalArray<Float_t>("chan", "nHits");
    locTreeBranchRegister.Register_FundamentalArray<Float_t>("x", "nHits");
    locTreeBranchRegister.Register_FundamentalArray<Float_t>("y", "nHits");
    locTreeBranchRegister.Register_FundamentalArray<Float_t>("E", "nHits");
    locTreeBranchRegister.Register_FundamentalArray<Float_t>("t", "nHits");
    locTreeBranchRegister.Register_FundamentalArray<Float_t>("integ", "nHits");
    locTreeBranchRegister.Register_FundamentalArray<Float_t>("ped", "nHits");
    locTreeBranchRegister.Register_FundamentalArray<Float_t>("peak", "nHits");
    locTreeBranchRegister.Register_FundamentalArray<Float_t>("integ", "nHits");
    locTreeBranchRegister.Register_FundamentalArray<Float_t>("integOpeak", "nHits");
    dTreeInterface->Create_Branches(locTreeBranchRegister);
  }
  TDirectory * dir_fcal_led = new TDirectoryFile("fcal_led", "fcal_led");
  dir_fcal_led->cd();
  h_XYGeo = new TH2F("XYGeo", ";row;column;Counts", 59, 0, 59, 59, 0, 59);
  h_XYGeo_w = new TH2F("XYGeo_w", ";row;column;E_{max}^{sum} [GeV]", 59, 0, 59, 59, 0, 59);
  h_E = new TH2F("E", ";channel;energy;Counts", 2800, 0, 2800, 1200, 0, 12.);
  h_ped = new TH2F("ped", ";channel;energy;Counts", 2800, 0, 2800, 400, 80., 120.);
  h_peak = new TH2F("peak", ";channel;energy;Counts", 2800, 0, 2800, 4096, -0.5, 4096.5);
  h_integ = new TH2F("integ", ";channel;energy;Counts", 2800, 0, 2800, 2000, 0., 200000.);
  h_integTot = new TH1F("integTot", ";Integral total;Counts", 2000, 0., 50e6);
  h_patterntot = new TH1F("patterntot", ";channel;Counts", 2800, 0, 2800);    
  h_integtot = new TH2F("integtot", ";channel;integ;Counts", 2800, 0, 2800, 3500, 0., 35000.);
  h_integOpeaktot = new TH2F("integOpeaktot", ";channel;integOpeak;Counts", 2800, 0, 2800, 1000, 0., 1000.);
  for (int i = 0; i < 6; i ++) {
    h_pattern[i] = new TH1F(Form("pattern_%d", i), ";channel;Counts", 2800, 0, 2800);
    h2d_integ[i] = new TH2F(Form("integ_%d", i), ";channel;integ;Counts", 2800, 0, 2800, 3500, 0., 35000);
    h_integOpeak[i] = new TH2F(Form("integOpeak_%d", i), ";channel;integOpeak;Counts", 2800, 0, 2800, 1000, 0., 1000.);
    h2d_peak[i] = new TH2F(Form("peak_%d", i), ";channel;peak;Counts", 2800, 0, 2800, 4096, -0.5, 4096.5);
    h2d_E[i] = new TH2F(Form("E_%d", i), ";channel;peak;Counts", 2800, 0, 2800, 4096, 0., 12.);
  }
  dir_fcal_led->cd("../");
  return;
}

//------------------
// brun
//------------------
void JEventProcessor_fcal_led::BeginRun(const std::shared_ptr<const JEvent>& event) {
  event->GetSingle(fcalGeom);
  dTreeFill.Fill_Single<Int_t>("rnb", event->GetRunNumber());
  return;
}

//------------------
// evnt
//------------------
void JEventProcessor_fcal_led::Process(const std::shared_ptr<const JEvent>& event) {
  auto lockService = DEvent::GetLockService(event);
  int m_nHits = 0;
  
  int m_chan[kMaxHits];
  float m_x[kMaxHits];
  float m_y[kMaxHits];
  float m_E[kMaxHits];
  float m_t[kMaxHits];
  float m_integ[kMaxHits];
  float m_ped[kMaxHits];
  float m_peak[kMaxHits];
  float m_integOpeak[kMaxHits];
  float m_eTot = 0;
  float m_integTot = 0;

  for (int i = 0; i < kMaxHits; i ++) {
    m_chan[i] = 0;
    m_x[i] = 0;
    m_y[i] = 0;
    m_E[i] = 0;
    m_t[i] = 0;
    m_integ[i] = 0;
    m_ped[i] = 0;
    m_peak[i] = 0;
    m_integOpeak[i] = 0;
  }
  
  bool is_FCAL_LED_trigger = false;
  auto locL1Triggers = event->Get<DL1Trigger>();
  auto locFCALHits = event->Get<DFCALHit>();
  auto locFCALDigiHits = event->Get<DFCALDigiHit>();
  auto locFCALGeometry = event->Get<DFCALGeometry>();
    
  int trig_bit[33];
  if (locL1Triggers.size() > 0) {
    for (unsigned int bit = 0; bit < 32; bit ++) {
      trig_bit[bit + 1] = (locL1Triggers[0]->trig_mask & (1 << bit)) ? 1 : 0;
    }
  }
  
  if (trig_bit[2] == 1) {
    is_FCAL_LED_trigger = true;
  }
  
  if(locFCALHits.size() > kMaxHits ) return;
  
  if (m_is_fcal_led_skim)
    is_FCAL_LED_trigger = true;

  if (is_FCAL_LED_trigger) {
        
    m_nHits = 0;
    m_eTot = 0;
    m_integTot = 0;
    if (m_do_tree) {
      dTreeFill.Fill_Single<Int_t>("evt", event->GetEventNumber());
    }
    for( vector< const DFCALHit* >::const_iterator hit = locFCALHits.begin(); hit != locFCALHits.end(); ++hit ){
      
      vector< const DFCALDigiHit* > digiHits;
      (**hit).Get( digiHits );
      if( digiHits.size() != 1 ) std::cout << "ERROR:  wrong size!! " << std::endl;
      
      const DFCALDigiHit& dHit = *(digiHits[0]);
      
      m_chan[m_nHits] = fcalGeom->channel( (**hit).row, (**hit).column );
      m_x[m_nHits] = (**hit).x;
      m_y[m_nHits] = (**hit).y;
      m_E[m_nHits] = (**hit).E;
      m_t[m_nHits] = (**hit).t;
      
      m_eTot += (**hit).E;
      
      m_ped[m_nHits] = (float)dHit.pedestal/dHit.nsamples_pedestal;
      m_peak[m_nHits] = dHit.pulse_peak - m_ped[m_nHits];
      m_integ[m_nHits] = dHit.pulse_integral - (m_ped[m_nHits]*dHit.nsamples_integral);
      
      m_integOpeak[m_nHits] = m_integ[m_nHits] / m_peak[m_nHits];
      
      m_integTot += m_integ[m_nHits];

      lockService->RootFillLock(this);
      h_XYGeo->Fill((**hit).row, (**hit).column);
      h_XYGeo_w->Fill((**hit).row, (**hit).column, (**hit).E);
      h_E->Fill(m_chan[m_nHits], (**hit).E);
      h_ped->Fill(m_chan[m_nHits], m_ped[m_nHits]);
      h_peak->Fill(m_chan[m_nHits], m_peak[m_nHits]);
      h_integ->Fill(m_chan[m_nHits], m_integOpeak[m_nHits]);
      lockService->RootFillUnLock(this); 
      if (m_do_tree) {
	dTreeFill.Fill_Array<Float_t>("chan", m_chan[m_nHits], m_nHits);
	dTreeFill.Fill_Array<Float_t>("x", m_x[m_nHits], m_nHits);
	dTreeFill.Fill_Array<Float_t>("y", m_y[m_nHits], m_nHits);
	dTreeFill.Fill_Array<Float_t>("E", m_E[m_nHits], m_nHits);
	dTreeFill.Fill_Array<Float_t>("t", m_t[m_nHits], m_nHits);
	dTreeFill.Fill_Array<Float_t>("ped", m_ped[m_nHits], m_nHits);
	dTreeFill.Fill_Array<Float_t>("peak", m_peak[m_nHits], m_nHits);
	dTreeFill.Fill_Array<Float_t>("integ", m_integ[m_nHits], m_nHits);
	dTreeFill.Fill_Array<Float_t>("integOpeak", m_integOpeak[m_nHits], m_nHits);
      }
      ++m_nHits;
    }
    if (m_do_tree) {
      dTreeFill.Fill_Single<Int_t>("nHits", m_nHits);
      dTreeFill.Fill_Single<Int_t>("eTot", m_integTot);
    }
    lockService->RootFillLock(this);
    h_integTot->Fill(m_integTot);
    
    for (int j = 0; j < m_nHits; j ++) {
      if (m_regions_inf[0] <= m_integTot && m_integTot <= m_regions_sup[5]) {
	h_patterntot->Fill(m_chan[j]);
	h_integtot->Fill(m_chan[j], m_integ[j]);
	h_integOpeaktot->Fill(m_chan[j], m_integOpeak[j]);
      }
    }
    
    for (int i = 0; i < 6; i ++) {
      for (int j = 0; j < m_nHits; j ++) {
	if (m_regions_inf[i] <= m_integTot && m_integTot <= m_regions_sup[i]) {
	  h_pattern[i]->Fill(m_chan[j]);
	  h2d_integ[i]->Fill(m_chan[j], m_integ[j]);
	  h_integOpeak[i]->Fill(m_chan[j], m_integOpeak[j]);
	  h2d_peak[i]->Fill(m_chan[j], m_peak[j]);
	  h2d_E[i]->Fill(m_chan[j], m_E[j]);
	}
      }
    }
    lockService->RootFillUnLock(this); 
    if (m_do_tree) {
      dTreeInterface->Fill(dTreeFill);
    }
  }
  return;
}

//------------------
// fini
//------------------
void JEventProcessor_fcal_led::Finish()
{
   delete dTreeInterface;
} 
//------------------
// erun
//------------------
void JEventProcessor_fcal_led::EndRun()
{
  return;
}


