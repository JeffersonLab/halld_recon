//
//    File: JEventProcessor_MilleFieldOn.h
// Creator: mstaib (on Linux egbert 2.6.32-642.6.2.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_MilleFieldOn_
#define _JEventProcessor_MilleFieldOn_

#include <JANA/JEventProcessor.h>
#include "Mille.h"

class JEventProcessor_MilleFieldOn : public JEventProcessor {
 public:
  JEventProcessor_MilleFieldOn();
  ~JEventProcessor_MilleFieldOn();

 private:
  void Init() override;
  void BeginRun(
      const std::shared_ptr<const JEvent>& event,
      int32_t runnumber);  ///< Called everytime a new run number is detected.
  void Process(const std::shared_ptr<const JEvent>& event,
                uint64_t eventnumber);  ///< Called every event.
  void EndRun() override;
                        ///< has been called.
  void Finish() override;
                        ///< been processed.
  Mille *milleWriter;
};

#endif  // _JEventProcessor_MilleFieldOn_
