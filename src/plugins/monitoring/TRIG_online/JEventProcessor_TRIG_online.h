// $Id$
//
//    File: JEventProcessor_TRIG_online.h
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_TRIG_online_
#define _JEventProcessor_TRIG_online_

#include <map>
using std::vector;
using std::map;

#include "TH1I.h"
#include "TH2I.h"

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

class JEventProcessor_TRIG_online:public JEventProcessor{
 public:
  JEventProcessor_TRIG_online();
  ~JEventProcessor_TRIG_online();


 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;

  bool timing;
  vector<uint32_t> dTrigBits;

  TH1I* h1trig_trgbits; 

  TH1I* h1trig_tot;
  TH2I* h2trig_fcalVSbcal;
  TH1I* h1trig_fcal_time;
  TH1I* h1trig_bcal_time;
  TH2I* h2trig_tfcalVStbcal;
  TH2I* h2trig_tfcalVSfcal;
  TH2I* h2trig_tbcalVSbcal;
 
  map<uint32_t, TH2I*> h2trigbits_fcalVSbcal;
  map<uint32_t, TH1I*> h1trigbits_fcal_time;
  map<uint32_t, TH1I*> h1trigbits_bcal_time;
  map<uint32_t, TH2I*> h2trigbits_tfcalVStbcal;
  map<uint32_t, TH2I*> h2trigbits_tfcalVSfcal;
  map<uint32_t, TH2I*> h2trigbits_tbcalVSbcal;
};

#endif // _JEventProcessor_TRIG_online_

