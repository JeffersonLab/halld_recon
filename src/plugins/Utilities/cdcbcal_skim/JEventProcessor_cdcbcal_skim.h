// $Id$
//
//    File: JEventProcessor_cdcbcal_skim.h
// Created: Mon feb 6 15:46:00 EST 2015
// Creator: wmcginle(on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_cdcbcal_skim_
#define _JEventProcessor_cdcbcal_skim_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>
#include "evio_writer/DEventWriterEVIO.h"
#include <TRACKING/DTrackFitter.h>

#include <vector>

using namespace std;

class JEventProcessor_cdcbcal_skim:public JEventProcessor{
 public:

  JEventProcessor_cdcbcal_skim();
  ~JEventProcessor_cdcbcal_skim();


 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  int num_epics_events;
};
#endif // _JEventProcessor_cdcbcal_skim_

