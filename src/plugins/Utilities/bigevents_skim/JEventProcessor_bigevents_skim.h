// $Id$
//
//    File: JEventProcessor_bigevents_skim.h
// Created: Thu May 12 08:01:59 EDT 2016
// Creator: zihlmann (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_bigevents_skim_
#define _JEventProcessor_bigevents_skim_

#include <JANA/JEventProcessor.h>
#include "evio_writer/DEventWriterEVIO.h"
#include <TRIGGER/DL1Trigger.h>

#include <CDC/DCDCDigiHit.h>
using namespace std;


class JEventProcessor_bigevents_skim:public JEventProcessor{
 public:
  JEventProcessor_bigevents_skim();
  ~JEventProcessor_bigevents_skim();


 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
};

#endif // _JEventProcessor_bigevents_skim_

