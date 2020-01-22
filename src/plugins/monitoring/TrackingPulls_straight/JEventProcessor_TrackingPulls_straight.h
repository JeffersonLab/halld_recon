// $Id$
//
//    File: JEventProcessor_TrackingPulls_straight.h
// Created: Wed Jan 22 09:28:45 EST 2020
// Creator: keigo (on Linux ifarm1401 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_TrackingPulls_straight_
#define _JEventProcessor_TrackingPulls_straight_

#include <JANA/JEventProcessor.h>

class JEventProcessor_TrackingPulls_straight : public jana::JEventProcessor {
 public:
  JEventProcessor_TrackingPulls_straight();
  ~JEventProcessor_TrackingPulls_straight();
  const char *className(void) {
    return "JEventProcessor_TrackingPulls_straight";
  }

 private:
  jerror_t init(void);  ///< Called once at program start.
  jerror_t brun(
      jana::JEventLoop *eventLoop,
      int32_t runnumber);  ///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop,
                uint64_t eventnumber);  ///< Called every event.
  jerror_t erun(void);  ///< Called everytime run number changes, provided brun
                        ///< has been called.
  jerror_t fini(void);  ///< Called after last event of last event source has
                        ///< been processed.
};

#endif  // _JEventProcessor_TrackingPulls_straight_
