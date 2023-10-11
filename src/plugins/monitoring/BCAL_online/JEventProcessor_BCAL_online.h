// $Id$
//
//    File: JEventProcessor_BCAL_online.h
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_BCAL_online_
#define _JEventProcessor_BCAL_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <BCAL/DBCALGeometry.h>

class JEventProcessor_BCAL_online:public JEventProcessor{
 public:
  JEventProcessor_BCAL_online();
  ~JEventProcessor_BCAL_online();

  bool REQUIRE_PHYSICS_TRIG;

  
  
  time_t  recentwalltime; 

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;

  
};

#endif // _JEventProcessor_BCAL_online_

