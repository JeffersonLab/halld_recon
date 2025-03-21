// $Id$
//
//    File: JEventProcessor_BCAL_saturation.h
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_BCAL_saturation_
#define _JEventProcessor_BCAL_saturation_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h>

class JEventProcessor_BCAL_saturation:public JEventProcessor{
 public:
  JEventProcessor_BCAL_saturation();
  ~JEventProcessor_BCAL_saturation();

  time_t  recentwalltime; 

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;

  int waveformCounterUS[2][4], waveformCounterDS[2][4];
};

#endif // _JEventProcessor_BCAL_saturation_

