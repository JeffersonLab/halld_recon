// $Id$
//
//    File: JEventProcessor_CDC_roc_hits.h
// Created: Wed Oct 22 2014
// Creator: Naomi Jarvis
//

#ifndef _JEventProcessor_CDC_roc_hits_
#define _JEventProcessor_CDC_roc_hits_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include "CDC/DCDCDigiHit.h"
#include "DAQ/Df125PulseIntegral.h"
#include "DAQ/Df125PulsePedestal.h"
#include "DAQ/Df125CDCPulse.h"
#include "DAQ/Df125Config.h"

#include <TDirectory.h>
#include <TH2.h>
#include <TFile.h>


// static root hist pointers for rootspy

  static TH1I *cdc_nevents;

  static TH2D *cdc_hits_roc25;
  static TH2D *cdc_hits_roc26;
  static TH2D *cdc_hits_roc27;
  static TH2D *cdc_hits_roc28;

  static TH2D *cdc_amp_roc25;
  static TH2D *cdc_amp_roc26;
  static TH2D *cdc_amp_roc27;
  static TH2D *cdc_amp_roc28;



class JEventProcessor_CDC_roc_hits:public JEventProcessor{
 public:
  JEventProcessor_CDC_roc_hits();
  ~JEventProcessor_CDC_roc_hits();


 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;


  int TSTART;  // start of window on le_time for time_selected and _t_ histos (units of sample/10)
  int TSTOP;   // end of window on le_time for time_selected and _t_ histos (units of sample/10)

  TH1D *cdc_sumamp_roc25 = NULL;
  TH1D *cdc_sumamp_roc26 = NULL;
  TH1D *cdc_sumamp_roc27 = NULL;
  TH1D *cdc_sumamp_roc28 = NULL;

  TH2D *cdc_netamp_roc25 = NULL;
  TH2D *cdc_netamp_roc26 = NULL;
  TH2D *cdc_netamp_roc27 = NULL;
  TH2D *cdc_netamp_roc28 = NULL;

  TH2D *cdc_time_roc25 = NULL;
  TH2D *cdc_time_roc26 = NULL;
  TH2D *cdc_time_roc27 = NULL;
  TH2D *cdc_time_roc28 = NULL;

  TH2D *cdc_ped_roc25 = NULL;
  TH2D *cdc_ped_roc26 = NULL;
  TH2D *cdc_ped_roc27 = NULL;
  TH2D *cdc_ped_roc28 = NULL;

  TH1D *cdc_time_selected = NULL;

  TH1D *cdc_sumamp_t_roc25 = NULL;
  TH1D *cdc_sumamp_t_roc26 = NULL;
  TH1D *cdc_sumamp_t_roc27 = NULL;
  TH1D *cdc_sumamp_t_roc28 = NULL;

  TH2D *cdc_amp_t_roc25 = NULL;
  TH2D *cdc_amp_t_roc26 = NULL;
  TH2D *cdc_amp_t_roc27 = NULL;
  TH2D *cdc_amp_t_roc28 = NULL;


};

#endif // _JEventProcessor_CDC_roc_hits_

