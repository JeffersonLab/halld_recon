// $Id$
//
//    File: JEventProcessor_cdc_goodtrack_skim.h
//

#ifndef _JEventProcessor_cdc_goodtrack_skim_
#define _JEventProcessor_cdc_goodtrack_skim_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>
#include "evio_writer/DEventWriterEVIO.h"
#include <TRACKING/DTrackFitter.h>

#include <vector>

using namespace std;

class JEventProcessor_cdc_goodtrack_skim:public JEventProcessor{
 public:

  JEventProcessor_cdc_goodtrack_skim();
  ~JEventProcessor_cdc_goodtrack_skim();


 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  int num_epics_events;
};
#endif // _JEventProcessor_cdc_goodtrack_skim_

