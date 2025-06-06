// $Id$
//
//    File: JEventProcessor_TAGM_online.h
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_TAGM_online_
#define _JEventProcessor_TAGM_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h>

class JEventProcessor_TAGM_online:public JEventProcessor{
 public:
  JEventProcessor_TAGM_online();
  ~JEventProcessor_TAGM_online();


 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
  std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_TAGM_online_

