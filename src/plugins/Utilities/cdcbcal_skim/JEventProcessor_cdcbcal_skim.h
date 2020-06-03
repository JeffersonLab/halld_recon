// $Id$
//
//    File: JEventProcessor_cdcbcal_skim.h
// Created: Mon feb 6 15:46:00 EST 2015
// Creator: wmcginle(on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_cdcbcal_skim_
#define _JEventProcessor_cdcbcal_skim_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>
#include "evio_writer/DEventWriterEVIO.h"
#include <TRACKING/DTrackFitter.h>

#include <vector>

using namespace jana;
using namespace std;

class JEventProcessor_cdcbcal_skim:public jana::JEventProcessor{
 public:

  JEventProcessor_cdcbcal_skim();
  ~JEventProcessor_cdcbcal_skim();
  const char* className(void){return "JEventProcessor_cdcbcal_skim";}


 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);	
  					///< Called after last event of last event source has been processed.

  int num_epics_events;
};
#endif // _JEventProcessor_cdcbcal_skim_

