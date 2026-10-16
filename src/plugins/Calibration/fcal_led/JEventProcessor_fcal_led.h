// $Id$
//
//    File: JEventProcessor_fcal_led.h
// Created: Fri May 19 12:39:24 EDT 2017
// Creator: mashephe (on Linux stanley.physics.indiana.edu 2.6.32-642.6.2.el6.x86_64 unknown)
//

#ifndef _JEventProcessor_fcal_led_
#define _JEventProcessor_fcal_led_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>


#include "FCAL/DFCALGeometry.h"
#include "FCAL/DFCALHit.h"
#include "FCAL/DFCALDigiHit.h"
#include "TRIGGER/DL1Trigger.h"
#include "TTree.h"
#include "TH2F.h"
#include "TH1F.h"
#include "ANALYSIS/DTreeInterface.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

class JEventProcessor_fcal_led:public JEventProcessor{
 public:
  JEventProcessor_fcal_led() {};
  ~JEventProcessor_fcal_led() {};
  const char* className(void){return "JEventProcessor_fcal_led";}

  enum { kMaxHits = 10000 };
  
 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  DTreeInterface* dTreeInterface;
  static thread_local DTreeFillData dTreeFill;
  
  const DFCALGeometry * fcalGeom = NULL;
  
  bool m_do_tree;
  bool m_is_fcal_led_skim;
  
  
  float m_LED_min1, m_LED_min2, m_LED_min3, m_LED_min4, m_LED_min5, m_LED_min6;
  float m_LED_max1, m_LED_max2, m_LED_max3, m_LED_max4, m_LED_max5, m_LED_max6;
  float m_regions_inf[6];
  float m_regions_sup[6];


  TH2F * h_XYGeo, * h_XYGeo_w;
  TH2F * h_E, * h_ped, * h_peak, * h_integ;
  TH1F * h_integTot, * h_patterntot;
  TH2F * h_integtot, * h_integOpeaktot;
  TH1F * h_pattern[6];
  TH2F * h2d_integ[6], * h_integOpeak[6], * h2d_peak[6], * h2d_E[6];
  

};

#endif // _JEventProcessor_fcal_led_

