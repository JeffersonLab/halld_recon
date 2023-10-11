// $Id$
//
//    File: JEventProcessor_MilleFieldOff.h
// Created: Tue Jan 17 19:32:32
// Creator: mstaib (on Linux egbert 2.6.32-642.6.2.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_MilleFieldOff_
#define _JEventProcessor_MilleFieldOff_

#include <JANA/JEventProcessor.h>
#include "Mille.h"

class JEventProcessor_MilleFieldOff : public JEventProcessor {
 public:
  JEventProcessor_MilleFieldOff();
  ~JEventProcessor_MilleFieldOff();

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event, int32_t runnumber);
  void Process(const std::shared_ptr<const JEvent>& event, uint64_t eventnumber);
  void EndRun() override;
  void Finish() override;

  Mille *milleWriter;
};

#endif  // _JEventProcessor_MilleFieldOff_
