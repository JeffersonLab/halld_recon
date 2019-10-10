// $Id$
//
//    File: JEventProcessor_DIRC_online.h
//

#ifndef _JEventProcessor_DIRC_online_
#define _JEventProcessor_DIRC_online_

#include <JANA/JEventProcessor.h>
#include <DIRC/DDIRCGeometry.h>

class JEventProcessor_DIRC_online:public jana::JEventProcessor{
 public:
  JEventProcessor_DIRC_online();
  ~JEventProcessor_DIRC_online();
  const char* className(void){return "JEventProcessor_DIRC_online";}


 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  const DDIRCGeometry* dDIRCGeometry;
  bool FillTimewalk;
};

#endif // _JEventProcessor_DIRC_online_

