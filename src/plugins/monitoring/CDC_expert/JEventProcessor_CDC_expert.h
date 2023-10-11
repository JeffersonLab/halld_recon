// $Id$
//
//    File: JEventProcessor_CDC_expert.h
// Created: Wed Oct 22 2014
// Creator: Naomi Jarvis
//

#ifndef _JEventProcessor_CDC_expert_
#define _JEventProcessor_CDC_expert_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>


class JEventProcessor_CDC_expert:public JEventProcessor{
 public:
  JEventProcessor_CDC_expert();
  ~JEventProcessor_CDC_expert();

	bool initialized_histograms;

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_CDC_expert_

