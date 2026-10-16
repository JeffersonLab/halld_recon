// $Id$
//
//    File: JEventProcessor_merge_rawevents.h
//

#ifndef _JEventProcessor_merge_rawevents_
#define _JEventProcessor_merge_rawevents_

#include <JANA/JEventProcessor.h>
#include "evio_writer/DEventWriterEVIO.h"

#include <vector>

class JEventProcessor_merge_rawevents:public JEventProcessor{
 public:

  JEventProcessor_merge_rawevents();
  ~JEventProcessor_merge_rawevents();

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  DEventWriterEVIO* dEventWriterEVIO;
};

#endif // _JEventProcessor_merge_rawevents_

