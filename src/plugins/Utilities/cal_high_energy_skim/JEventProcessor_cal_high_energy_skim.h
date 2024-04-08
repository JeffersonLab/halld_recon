// $Id$
//
//    File: JEventProcessor_cal_high_energy_skim.h
//

#ifndef _JEventProcessor_cal_high_energy_skim_
#define _JEventProcessor_cal_high_energy_skim_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include "evio_writer/DEventWriterEVIO.h"

#include <TH1F.h>

#include <vector>

using namespace std;

class JEventProcessor_cal_high_energy_skim:public JEventProcessor{
 public:

  JEventProcessor_cal_high_energy_skim();
  ~JEventProcessor_cal_high_energy_skim();


 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  double MIN_BCAL_E;
  double MIN_FCAL_E;
  double MIN_TOTAL_CAL_E;
 
  //int WRITE_EVIO;
  int MAKE_DIAGNOSTICS;
  int num_epics_events;

  TH1F *h_BCAL_shen;
  TH1F *h_FCAL_shen;

};
#endif // _JEventProcessor_cal_high_energy_skim_

