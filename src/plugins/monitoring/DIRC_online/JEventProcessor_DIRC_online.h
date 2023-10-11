// $Id$
//
//    File: JEventProcessor_DIRC_online.h
//

#ifndef _JEventProcessor_DIRC_online_
#define _JEventProcessor_DIRC_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <DIRC/DDIRCGeometry.h>

class JEventProcessor_DIRC_online:public JEventProcessor{
 public:
  JEventProcessor_DIRC_online();
  ~JEventProcessor_DIRC_online();


 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;

  bool FillTimewalk;
  
};

#endif // _JEventProcessor_DIRC_online_

