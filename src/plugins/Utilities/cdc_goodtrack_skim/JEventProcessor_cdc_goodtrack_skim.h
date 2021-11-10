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

using namespace jana;
using namespace std;

class JEventProcessor_cdc_goodtrack_skim:public jana::JEventProcessor{
 public:

  JEventProcessor_cdc_goodtrack_skim();
  ~JEventProcessor_cdc_goodtrack_skim();
  const char* className(void){return "JEventProcessor_cdc_goodtrack_skim";}


 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);	
  					///< Called after last event of last event source has been processed.

  int num_epics_events;
};
#endif // _JEventProcessor_cdc_goodtrack_skim_

