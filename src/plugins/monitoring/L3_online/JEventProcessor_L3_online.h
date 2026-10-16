// $Id$
//
//    File: JEventProcessor_L3_online.h
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_L3_online_
#define _JEventProcessor_L3_online_

#include <JANA/JEventProcessor.h>


class JEventProcessor_L3_online:public JEventProcessor{
 public:
  JEventProcessor_L3_online();
  ~JEventProcessor_L3_online();
  SetTypeName("JEventProcessor_L3_online");


 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
};

#endif // _JEventProcessor_L3_online_

