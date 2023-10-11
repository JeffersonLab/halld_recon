// $Id$
//
//    File: JEventProcessor_CDC_expert_2.h
// Created: Wed Oct 22 2014
// Creator: Naomi Jarvis
//

#ifndef _JEventProcessor_CDC_expert_2_
#define _JEventProcessor_CDC_expert_2_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>


using namespace std;


#include "CDC/DCDCHit.h"
#include "CDC/DCDCDigiHit.h"
#include "DAQ/Df125WindowRawData.h"
#include "DAQ/Df125CDCPulse.h"
#include "DAQ/Df125Config.h"
#include "TRIGGER/DTrigger.h"

#include <TDirectory.h>
#include <TH2.h>
#include <TH1.h>


class JEventProcessor_CDC_expert_2:public JEventProcessor{
 public:
  JEventProcessor_CDC_expert_2();
  ~JEventProcessor_CDC_expert_2();


 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;


  //DO NOT MAKE THESE STATIC GLOBAL EVER AGAIN!!

  // root hist pointers

  TH1D *cdc_e = NULL;
  TH2D *cdc_e_vs_n = NULL;

  TH1D *cdc_t = NULL;
  TH2D *cdc_t_vs_n = NULL;

  TH1I *cdc_rt = NULL;
  TH2I *cdc_rt_vs_n = NULL;

  TH1I *cdc_amp = NULL;
  TH2I *cdc_amp_vs_n = NULL;

  TH1I *cdc_rt_qf0 = NULL;

  TH1I *cdc_qf = NULL;
  TH2I *cdc_qf_vs_n = NULL;
  TH2I *cdc_qf_vs_a = NULL;
  TH2I *cdc_qf_vs_rt = NULL;


  //TH2D *cdc_e_ring[29];
  TH2D *cdc_t_ring[29];

  TH2D *cdc_e_vs_t;
  TH2D *cdc_e_vs_t_ring[29];

  TH2I *cdc_int_vs_raw_t;
  TH2I *cdc_int_vs_raw_t_ring[29];

  TH2I *cdc_o_overflow;
  TH1I *cdc_ped_overflow;
  TH1I *cdc_raw_t_overflow;

  TH2I *cdc_o_badt;

  TH2I *cdc_ped_ring[29];
  TH1I *cdc_ped_badt;

  TH2I *cdc_raw_t_ring[29];
  TH1I *cdc_raw_t_badt;

  TH2I *cdc_amp_ring[29];
  TH1I *cdc_amp_badt;


  TH2I *cdc_intpp_ring[29];
  //TH2I *cdc_int_ring[29];

  TH2I *cdc_initped_ring[29];

  TH2I *cdc_initped_roc25;
  TH2I *cdc_initped_roc26;
  TH2I *cdc_initped_roc27;
  TH2I *cdc_initped_roc28;

  TH2I *cdc_ped_roc25;
  TH2I *cdc_ped_roc26;
  TH2I *cdc_ped_roc27;
  TH2I *cdc_ped_roc28;

  TH2I *cdc_amp_roc25;
  TH2I *cdc_amp_roc26;
  TH2I *cdc_amp_roc27;
  TH2I *cdc_amp_roc28;

  TH2I *cdc_what_is_n;

};

#endif // _JEventProcessor_CDC_expert_2_

